#include "servidorHTTP.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include <syslog.h>


/* Muestra el mensaje de error y termina el programa con EXIT_FAILURE */
void error(char *msg) {
	log_error(msg);
    perror(msg);
    exit(EXIT_FAILURE);
}

/* Para loggear errores en el system log */
void log_error(char *msg) {
	openlog ("servidorHTTP", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);
	syslog (LOG_ERR, msg);
	closelog ();
}

/* Para loggear informacion en el system log */
void log_info(char *msg) {
	openlog ("servidorHTTP", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);
	syslog (LOG_INFO, msg);
	closelog ();
}

/* Muestra mensaje de ayuda */
void ayuda() {
   printf("Modo de uso: ./servidorHTTP [servidor][:puerto] [-h]\n \n");
   printf("\t[servidor]: \tDireccion IP del servidor. (Default: localhost) \n");
   printf("\t[:puerto]: \tPuerto del servidor. (Default: 80)\n");
   printf("\t[-h]: \t\tAyuda por pantalla (este mensaje). \n");
   printf("\n");
   printf("Si no se especifica servidor o puerto, la direccion \n");
   printf("por defecto sera 127.0.0.1 y el puerto por defecto sera 80.\n\n");
   exit(EXIT_SUCCESS);
}

/* Manejador de señales */
void signalHandler(int sig) {
	if (sig == SIGUSR1) {  
		exit(EXIT_SUCCESS); 
	} 
}

/* Verifica si una IP es valida */
int verificarIP(char * servidor){
	int i=inet_addr(servidor);
	if(i==INADDR_NONE) { 
		return 0;
	} else {
		return 1;
	}
}

/* Verifica si un puerto es valido */
int verificarPuerto(char * puerto){
	int p = atoi(puerto);
	if (p>=1 && p<=65535) {
		return 1;
	} else {
		return 0;
	}
}

int main(int argc, char *argv[]) {
	// Reviso si me ingresaron un -h para mostrar ayuda
	// Si asi fuera, se muestra la ayuda y se termina el programa
	int i;
	for (i = 1; i <= (argc - 1); i++) {
		if (strcmp("-h", argv[i]) == 0) {
			ayuda();
		}
	}
	
	// Configuración para el manejo de señales
	signal(SIGUSR1, (void *)signalHandler);
	signal(SIGUSR2, (void *)signalHandler);
    signal(SIGCHLD, SIG_IGN);
	signal(SIGABRT, (void *)signalHandler);
	signal(SIGHUP, (void *)signalHandler);
	signal(SIGINT, (void *)signalHandler);
	signal(SIGQUIT, (void *)signalHandler);
	signal(SIGTERM, (void *)signalHandler);
	
	// Correr el programa en background. Parametros (1,1) para no cambiar path ni stdin/out/err!
	if (daemon(1,1) < 0) { error(ERROR_DAEMON);}
	
	// En este punto del programa si esta corriendo es porque no preguntaron por ayuda
	char * servidor = NULL;
	char * puerto = NULL; 
	// Asigno Servidor:Puerto, si existiera
	// En caso de que no ingreso argumentos, uso el default
	if (argc == 1) {
		// No pasaron argumentos (uso default!)
		servidor = strdup(LOCALHOST); 	// 127.0.0.1
		puerto = strdup(WEBPORT);		// 80
	} else {
		char * str = argv[1];
		
		// Split Servidor:Puerto
		servidor = strtok(str, ":");
		puerto = strtok(NULL, ":");
		
		if (strstr(str,":"))  {
			// Ingreso :puerto pero no ingreso servidor
			puerto = servidor;
			servidor = strdup(LOCALHOST); 
		} else {
			// Ingreso servidor:puerto o solo servidor
					
			// Si solo me pasaron servidor (y no puerto)
			// entonces asigno el puerto por defecto (80)
			if (puerto == NULL) { puerto= strdup(WEBPORT); }
		}
	}
	
	log_info(servidor);
	log_info(puerto);
	
	if (servidor == NULL || puerto == NULL) {
		error(ERROR_INPUT_DATOS);
	} else 	if (!verificarIP(servidor) || !verificarPuerto(puerto)) {
		error(ERROR_IP_PORT);
	}

	int sockfd, newsockfd, portno, clilen, pid;
	struct sockaddr_in serv_addr, cli_addr;
	
	// creo un socket TCP y obtengo el File Descriptor
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd < 0) 
		error(ERROR_ABRIR_SOCKET);
	
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(puerto);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(servidor); 
	serv_addr.sin_port = htons(portno);
	
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		error(ERROR_BIND_SOCKET);
	
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	while (1) {
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0) 
			error(ERROR_ACCEPT_SOCKET);
		
		pid = fork();
		
		if (pid < 0) {
			error(ERROR_FORK);
		} else if (pid == 0)  {
			// Soy hijo, me desligo del socket paterno y atiendo el pedido
			close(sockfd);
			atenderPedido(newsockfd);
			exit(EXIT_SUCCESS);
		} else  { 
			// Soy padre, me desligo del socket de mi hijo 
			// y sigo atendiendo pedidos.
			close(newsockfd);
		}
	} 
	// No deberia llegar aca
	return 0; 
}

/* Parsea el mensaje dado en buffer y retorna el tipo de mensaje, 
 * la ruta del archivo y el protocolo usado.
 * No se encarga de hacer chequeos sobre la correctitud del mensaje.
 *  */
void parseMsg (char * buf, char ** tipoMsg, char ** ruta, char ** protocolo) { 
   	char * token = strtok(buf, "\n\r");
    *tipoMsg = strtok(token," ");
    *ruta = strtok(NULL," ");
    *protocolo = strtok(NULL," ");    	
}

int archivoExiste(char * archivo) {
	int rval = access (archivo, F_OK);
	if (rval == 0) 
			return 1;
	else return 0;
}

int archivoAbrible(char * archivo) {
	int rval = access (archivo, R_OK);
	if (rval == 0) 
			return 1;
	else return 0;
}

long archivoSize(char * archivo) {
	FILE *file = fopen(archivo, "rb");
	if (!file) error(ERROR_ABRIR_ARCHIVO); 
	fseek(file, 0, SEEK_END);
	long f_size = ftell(file);
	fseek(file, 0, SEEK_SET);
	fclose(file);
	return f_size;
}

/* Revisa si una cadena dada es GET */
int esGet(char * msg){
	if (strcmp("GET", msg) == 0) 
		return 1;
	else return 0;
}

/* Revisa si una cadena dada es barra (/) */
int esBarra(char * msg){
	if (strcmp(msg,"/")==0)
		return 1;
	else return 0;
}

/* Manda headers a traves del socket, 
 * segun el tipo de respuesta y de contenido */
void mandarHeader(int sock, char * resp){
	if (resp!=NULL) {
		if (send(sock,resp,strlen(resp),0) < 0) {
			error(ERROR_SEND_SOCKET);
		}
	}
}

/* Manda headers a traves del socket, 
 * segun el tipo de respuesta y de contenido */
void mandarHeaders(int sock, char * tipoResp, char * tipoCont){
	mandarHeader(sock,tipoResp);
	mandarHeader(sock,tipoCont);
}

/* Manda un header 4xx con un rechazo,
 * ademas le manda contenido en HTML
 * para mostrar una pagina de Error. */
void mandarRechazo(int sock, char * tipoResp, char * titulo, char * mensaje){
	mandarHeaders(sock,tipoResp,CT_HTML);	
	// Preparo un mensaje HTML con el error 4xx o 5xx
	char * rta = strdup("<html><body><title>");
	strcat(rta,titulo);
	strcat(rta,"</title><h1>");
	strcat(rta,titulo);
	strcat(rta,"</h1><p>");
	strcat(rta,mensaje);
	strcat(rta,"</p></body></html>");
	mandarHeader(sock,rta);
}

/* Dado un socket y un archivo, se manda dicho archivo 
 * en modo binario a traves del socket */
void mandarArchivo(int sock, char * archivo){
	size_t lfile = archivoSize(archivo);
	char * bufferSalida = malloc(lfile);

	FILE *file = fopen(archivo, "rb");
	if (!file) error(ERROR_ABRIR_ARCHIVO); 
	int c; size_t t = 0;

	int nro,n;
	while(!feof(file))
	{
		nro = fread(bufferSalida,1,128,file);
		n = send(sock,bufferSalida,nro,0);
		if (n < 0) error(ERROR_SEND_SOCKET);
	}
	free(bufferSalida);
}

/* Revisa si una cadena es .html o .htm */
int esHTML(char * archivo){
	char * arch = getExtension(minusculas(archivo));
	if ((strcmp(arch,".html")==0) || (strcmp(arch,".htm")==0))
		return 1;
	else return 0;
}

/* Revisa si una cadena es .jpg */
int esJPG(char * archivo){
	char * arch = getExtension(minusculas(archivo));
	if (strcmp(arch,".jpg")==0)
		return 1;
	else return 0;
}

/* Revisa si una cadena es .gif */
int esGIF(char * archivo){
	char * arch = getExtension(minusculas(archivo));
	if (strcmp(arch,".gif")==0)
		return 1;
	else return 0;
}

/* Revisa si una cadena es .png */
int esPNG(char * archivo){
	char * arch = getExtension(minusculas(archivo));
	if (strcmp(arch,".png")==0)
		return 1;
	else return 0;
}

/* Revisa si una cadena es .php */
int esPHP(char * archivo){
	char * arch = getExtension(minusculas(archivo));
	if (strcmp(arch,".php")==0)
		return 1;
	else return 0;
}

/* Dado una cadena, obtiene su extension (si la tuviese) */
char * getExtension(char* archivo) {
	char *aux, *ext=strstr(archivo,".");
	aux=ext; 
	while (ext !=NULL) {		
		aux= ext;
		ext = strstr(ext+1, ".");
	}
	return aux;
}

/* Funcion para agregar un caracter al final de una cadena */
char * appchr(char * str, const char chr) {
	size_t len = strlen(str);;
	char * ptr = malloc(len+2);
	strcpy(ptr,str);
	ptr[len] = chr;
	ptr[len+1] = '\0';
	return ptr;
}

/* Recibe el mensaje de un socket hasta que se lean dos "enters" seguidos.
 * Devuelve un puntero al principio de lo leido (buffer) */
char * recibirMensaje(int sock) {
	char * buffer = malloc(sizeof(char));;
	char * letra  = malloc(sizeof(char));
	char a,b,c,d;
	int n;
	a = 1 ; b = 1; c = 1; d = 1; n = 1;
	int termine = 0;
	
	while (!termine && n>0){
	   n = recv(sock,letra,1,0);
	   if (n < 0) error(ERROR_RECV_SOCKET);
	   d = letra[0];
	   buffer = appchr(buffer,d);
	   // Si veo dos enters seguidos activo el flag de terminacion
	   if ((a == 10 && b == 13 && c == 10 && d == 13) || (a == 13 && b == 10 && c == 13 && d == 10)){
		   termine = 1;
	   } else {
		   a = b;
		   b = c;
		   c = d;
	   }	   
	}
	free(letra);
	return buffer;
}

/* Dada una cadena de caracteres, devuelve la misma cadena pero en minusculas */
char * minusculas(char * str){
	int i;
	char * rta = malloc(strlen(str));
	for(i = 0; str[i]; i++){
		rta[i] = tolower(str[i]);
	}
	return rta;
}

/* Dada una ruta mediante archivo, obtiene el nombre del archivo
 * y los argumentos (si los hubiera), los separa y retorna
 * la ruta al indice (mediante el primer parametro)
 * y sus argumentos mediante el segundo parametro */
void verificarPHP(char ** archivo, char ** argumentos){
	char * arch = strdup(*archivo);
	char * ruta = strtok(arch, " ?\n\r");
    * argumentos = strstr(*archivo, "?");
	* archivo = ruta;
}

/* Se encarga de la parte PHP. Hace el fork, el hijo el exec(php-chi) 
 * y el padre envia el mensaje generado por el hijo */ 
void procesarPHP(int sock, char * archivo, char * parametros){
	int pipefd[2];
	if (pipe(pipefd) < 0) { error(ERROR_PIPE); }
	
	pid_t pid = fork();

	if (pid == 0) {
		close(pipefd[0]);    // close reading end in the child
		dup2(pipefd[1], 1);  // mando standart output al pipe
		dup2(pipefd[1], 2);  // mando standart error al pipe
		close(pipefd[1]);    // this descriptor is no longer needed

		char * query;
		if (parametros!=0) {
			// Cargo en la variable de entorno QUERY_STRING
			// los parametros que me hayan pasado
			query = malloc(13+strlen(parametros));
			strcpy(query,"QUERY_STRING=");
			strcat(query,parametros+1);
			putenv(query);
		} else { 
			// No me pasaron parametros, "reseteo el query string"
			putenv("QUERY_STRING="); 
		}
				
		// Cargo en la variable de entorno SCRIPT_FILENAME
		// el archivo que este solicitando el usuario
		char script [16+strlen(archivo)];
		strcpy(script,"SCRIPT_FILENAME=");		
		strcat(script,archivo);
		putenv(script);

		execlp("php-cgi","php-cgi",NULL);
	} else if (pid > 0) {
		// Soy el padre
		char buf[1];
		close(pipefd[1]);  // close the write end of the pipe in the parent
		int n;
		
		// Espero a que termine mi hijo
		int returnStatus;
		waitpid(pid,&returnStatus,0);
		
		char * buffer = malloc(sizeof(char));;
	
		while (read(pipefd[0], buf, sizeof(buf)) != 0)
		{
			// Cargo lo que me respondio php-cgi en un buffer
			buffer = appchr(buffer,buf[0]);
		}	
		// Lo mando
		mandarHeader(sock,RTA_200);		
		mandarHeader(sock,buffer);		
		free(buffer);	
	}
	else if (pid < 0) { error(ERROR_FORK); }
}

/* Metodo principal que se encarga de atender un pedido mediante un socket dado */
void atenderPedido(int sock) {
	int n;
	
	char * tipoMsg = NULL;
	char * ruta = NULL;
	char * protocolo = NULL;
   
	char * buffer = recibirMensaje(sock);

    // Analizo el mensaje (la primera linea es la que importa en realidad)
    // Obtengo el tipo de metodo, la ruta y el protocolo utilizado.
    parseMsg(buffer, &tipoMsg, &ruta, &protocolo);
	
	char * archivo = NULL;
	char * parametros = NULL;
	if (esBarra(ruta)) {
		// Si me pasaron / tengo que buscar si tengo index.html, index.htm, o index.php
		// Sobreescribir sobre el valor de la variable archivo
		if (archivoExiste("index.html")) {
			archivo = strdup("index.html");
		} else if (archivoExiste("index.htm")) {
				archivo = strdup("index.htm");
			} else if (archivoExiste("index.php")) {
					archivo = strdup("index.php");
				}
	} else {
		archivo = ruta+1;
	}
	
	// Verifico si el archivo es PHP y separo sus parametros ("desgloso")
	// Esto lo hago en este punto porque luego se hara el chequeo
	// de la existencia del archivo (y necesitamos unicamente el nombre del archivo)
	verificarPHP(&archivo,&parametros);

    if (!ruta || !tipoMsg || !protocolo) {
		// Me mandaron mal la request (alguno de los elementos del primer renglon es vacio (NULL);
		mandarRechazo(sock,RTA_400,"400 Bad Request", "The request sent didn't have the correct syntax.");
	} else {
		// Me mandaron un request que "puedo entender"
		// Trato de interpretarlo y trabajarlo
		if (esGet(tipoMsg)) {	
			if(archivoExiste(archivo)) {
				// El archivo existe
				if (archivoAbrible(archivo)) {
					// El archivo se puede abrir			
					if (esHTML(archivo)) {
						// Es HTML o HTM
						mandarHeaders(sock,RTA_200,CT_HTML);
						mandarArchivo(sock,archivo);
					} else if (esJPG(archivo) || esPNG(archivo) || esGIF(archivo)) {
							// Es JPG, GIF o PNG
							if (esJPG(archivo)) {
								mandarHeaders(sock,RTA_200,CT_JPG);
							}
							else if (esGIF(archivo)) {
									mandarHeaders(sock,RTA_200,CT_GIF);
								}
								else { 
									mandarHeaders(sock,RTA_200,CT_PNG); 
								}
							// Ya mande los headers, ahora mando el archivo
							mandarArchivo(sock,archivo);
					} else if (esPHP(archivo)) {
							// es PHP
							procesarPHP(sock,archivo,parametros);				
						} else {
							// No es un tipo valido (extension desconocida) pero existe el archivo
							// Tomar una decision de diseño. Por ejemplo, mandar un 200 OK y el contenido del archivo
							// Ojo con esto, podria influir en la "seguridad" del servidor
							mandarRechazo(sock,RTA_403,"403 Forbidden", "Not allowed to access the resource and authorization will not help.");
						}												
				} else {
					// Archivo no se puede abrir. Mando Error 403
					mandarRechazo(sock,RTA_403,"403 Forbidden", "Not allowed to access the resource and authorization will not help.");
				}		
			} else {
				// Archivo no existe, Mando Error 404
				mandarRechazo(sock,RTA_404,"404 Not Found", "The requested file was not found.");
			}
		} else {
			// Metodo no permitido, mando Error 501
			mandarRechazo(sock,RTA_501,"501 Not Implemented", "The requested method is not implemented.");
		}
	}
	free(buffer);
	
}
