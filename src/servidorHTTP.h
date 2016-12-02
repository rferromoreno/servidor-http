#define LOCALHOST "127.0.0.1"
#define WEBPORT "80"

// Mensajes de error
#define ERROR_ABRIR_SOCKET "Error al abrir el socket \n"
#define ERROR_BIND_SOCKET "Error al ligar el socket \n"
#define ERROR_ACCEPT_SOCKET "Error al aceptar la conexion \n"
#define ERROR_RECV_SOCKET "Error al leer del socket \n"
#define ERROR_SEND_SOCKET "Error al escribir en el socket \n"
#define ERROR_FORK "Error al hacer fork \n"
#define ERROR_ABRIR_ARCHIVO "Error al abrir el archivo \n"
#define ERROR_PIPE "Error al hacer el pipe \n"
#define ERROR_DAEMON "Error al poner el programa en background \n"
#define ERROR_UNEXPECTED_END "El servidor finalizo de manera inesperada \n"
#define ERROR_IP_PORT "La IP o el puerto ingresado es invalido \n"
#define ERROR_INPUT_DATOS "Error en el ingreso de datos \n"

// Mensajes de respuesta HTTP https://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
#define RTA_200 "HTTP/1.0 200 OK \n"
#define RTA_400 "HTTP/1.0 400 Bad Request \n"
#define RTA_403 "HTTP/1.0 403 Forbidden \n"
#define RTA_404 "HTTP/1.0 404 Not Found \n"
#define RTA_501 "HTTP/1.0 501 Not Implemented \n"

// Tipos de Contenido usados en el proyecto
#define CT_HTML "Content-type: text/html \n\n"
#define CT_JPG "Content-type: image/jpeg \n\n"
#define CT_PNG "Content-type: image/png \n\n"
#define CT_GIF "Content-type: image/gif \n\n"

#ifndef SERVIDORHTTP_H_   /* Include guard */
#define SERVIDORHTTP_H_

/** error:
 * Muestra el mensaje de error y termina el programa con EXIT_FAILURE 
 * DE: 	Mensaje (string) 
 * */
void error(char *msg);

/** log_error:
 * Registra el error dado en el system log de servidor HTTP. 
 * DE: 	Mensaje (string) 
 * */
void log_error(char *msg);

/** log_info:
 * Registra la informacion dada en el system log de servidor HTTP. 
 * DE: 	Mensaje (string) 
 * */
void log_info(char *msg);

/** ayuda:
 * Muestra el mensaje de ayuda al usuario y termina el programa con EXIT_SUCCESS 
 * */
void ayuda();

/** signalHandler:
 * Método que se encarga de manejar las señales recibidas.
 * DE: 	Signal (int), el código de la señal a manejar.
 * */
void signalHandler(int sig);

/** verificarIP:
 * Dada una IP, verifica si la IP corresponde a una IPv4 valida.
 * DE: 	Servidor (string), la IP a analizar.
 * DS:	1 si la IP es valida, 0 en caso contrario.
 * */
int verificarIP(char * servidor);

/** verificaPuerto:
 * Dado un puerto, verifica si el puerto es valido (esta entre 1 y 65535).
 * DE: 	Puerto (string), el puerto a analizar..
 * DS:	1 si el puerto es valido, 0 en caso contrario.
 * */
int verificarPuerto(char * puerto);

/** parseMsg: 
 * Analiza el mensaje brindado en el buffer y retorna cuál es el tipo de mensaje,
 * la ruta del archivo y el protocolo utilizado. No se encarga de hacer chequeos
 * en cuanto a la correctitud del mensaje.
 * DE:	Buf, el buffer de entrada que posee el mensaje.
 * DS: 	TipoMsg, Ruta y Protocolo, los datos de salida según el análisis sintáctico.
 *  */
void parseMsg (char * buf, char ** tipoMsg, char ** ruta, char ** protocolo);

/** archivoExiste:
 * Dada la ruta de un archivo, retorna verdadero si el archivo existe y falso en caso contrario. 
 * DE: 	Archivo (string), la ruta del archivo.
 * DS: 	1 si el archivo existe, 0 en caso contrario.
 * */
int archivoExiste(char * archivo);

/** archivoAbrible:
 * Dada la ruta de un archivo, retorna verdadero si el archivo se puede abrir y falso en caso contrario. 
 * DE: 	Archivo (string), la ruta del archivo.
 * DS: 	1 si el archivo se puede abrir, 0 en caso contrario.
 * */
int archivoAbrible(char * archivo);

/** archivoSize:
 * Dada la ruta de un archivo, retorna el tamaño del contenido del archivo. 
 * DE: 	Archivo (string), la ruta del archivo.
 * DS: 	Size (long), el tamaño del contenido del archivo.
 * */
long archivoSize(char * archivo);

/** esGet:
 * Dada una cadena de texto, analiza si la misma es un "GET"
 * DE: 	Mensaje (string), la cadena de texto.
 * DS: 	1 si la cadena es "GET", 0 en caso contrario.
 * */
int esGet(char * msg);

/** esBarra:
 * Dada una cadena de texto, analiza si la misma es una barra "/"
 * DE: 	Mensaje (string), la cadena de texto.
 * DS: 	1 si la cadena es una barra "/", 0 en caso contrario.
 * */
int esBarra(char * msg);

/** mandarHeader:
 * Dada una cadena de texto y un socket, envia el texto a traves de ese socket.
 * DE: 	Sock (int), el socket asociado por donde mandar el mensaje.
 * 		Resp (string), el mensaje a ser enviado a traves del socket.
 * */
void mandarHeader(int sock, char * resp);

/** mandarHeaders:
 * Método para encapsular y mandar 2 mensajes a traves de un socket.
 * Hace dos llamadas a el metodo "mandarHeader", con tipoResp y con tipoCont.
 * Normalmente se usa para mandar un tipo de respuesta (status)
 * y el tipo de contenido a ser enviado (content type).
 * DE: 	Sock (int), el socket asociado por donde mandar el mensaje.
 * 		TipoResp (string), el tipo de respuesta a enviar.
 * 		TipoCont (string), el tipo de contenido a enviar.
 * */
void mandarHeaders(int sock, char * tipoResp, char * tipoCont);

/** mandarRechazo:
 * Dado un socket, un tipo de respuesta, un titulo y un mensaje,
 * el método se encarga de mandar una respuesta del estado de tipoResp.
 * Y además se encarga de crear una página de contenido HTML con el 
 * título y el mensaje otorgado. El método normalmente se usa para mandar
 * respuestas de tipo 4XX o 5XX y mostrar de forma amigable una página,
 * para los que esten haciendo solicitudes mediante un navegador web. 
 * DE: 	Sock (int), el socket asociado por donde mandar el mensaje.
 * 		TipoResp (string), el tipo de respuesta a ser enviado.
 * 		Titulo (string), título a ser mostrado en el contenido HTML.
 * 		Mensaje (string), mensaje a ser mostrado en el contenido HTML.
 * */
void mandarRechazo(int sock, char * tipoResp, char * titulo, char * mensaje);

/** mandarArchivo:
 * Dado un socket y la ruta de un archivo, manda el archivo
 * en modo binario a traves del socket.
 * DE: 	Socket (int), el socket asociado para mandar el archivo.
 * 		Archivo (string), la ruta del archivo.
 * */
void mandarArchivo(int sock, char * archivo);

/** esHTML:
 * Dada una cadena de texto referente a la ubicación de un archivo, 
 * revisa si el archivo tiene la extension HTML o HTM.
 * DE: 	Archivo (string), la ruta del archivo.
 * DS: 	1 si el archivo es HTML/HTM, 0 en caso contrario.
 * */
int esHTML(char * archivo);

/** esJPG:
 * Dada una cadena de texto referente a la ubicación de un archivo, 
 * revisa si el archivo tiene la extension JPG.
 * DE: 	Archivo (string), la ruta del archivo.
 * DS: 	1 si el archivo es JPG, 0 en caso contrario.
 * */
int esJPG(char * archivo);

/** esGIF:
 * Dada una cadena de texto referente a la ubicación de un archivo, 
 * revisa si el archivo tiene la extension GIF.
 * DE: 	Archivo (string), la ruta del archivo.
 * DS: 	1 si el archivo es GIF, 0 en caso contrario.
 * */
int esGIF(char * archivo);

/** esPNG:
 * Dada una cadena de texto referente a la ubicación de un archivo, 
 * revisa si el archivo tiene la extension PNG.
 * DE: 	Archivo (string), la ruta del archivo.
 * DS: 	1 si el archivo es PNG, 0 en caso contrario.
 * */
int esPNG(char * archivo);

/** esPHP:
 * Dada una cadena de texto referente a la ubicación de un archivo, 
 * revisa si el archivo tiene la extension PHP.
 * DE: 	Archivo (string), la ruta del archivo.
 * DS: 	1 si el archivo es PHP, 0 en caso contrario.
 * */
int esPHP(char * archivo);

/** getExtension:
 * Dada una cadena de texto, obtiene su extension (si la tuviese).
 * DE: 	Archivo (string), la ruta del archivo para obtener su extensión.
 * DS:	Retorna un puntero a un string con la extensión, o NULL en caso de no
 * 		poseer una extensión reconocible.
 * */
char * getExtension(char* archivo);

/** appchr:
 * Dada una cadena de texto y un caracter, agrega dicho caracter
 * al final de la cadena de texto (append).
 * DE: 	Str (string), la cadena de texto.
 * 		Chr (char), la letra para agregar al final de la cadena.
 * DS:	Retorna un puntero a la primera posición de la cadena de texto resultante.
 * */
char * appchr(char * str, const char chr);

/** recibirMensaje:
 * Dado un socket, se hace una lectura a través de dicho socket hasta que
 * se lean dos enter seguidos (CR-LF o LF-CR). 
 * Luego, se retorna un string con el contenido de lo leído.
 * DE: 	Socket (int), el socket asociado para hacer la lectura.
 * DS:	Retorna un puntero a la primera posición de la cadena de texto leída.
 * */
char * recibirMensaje(int sock);

/** minusculas:
 * Dada una cadena de caracteres, devuelve la misma cadena pero convertida a minúsculas.
 * DE: 	Str (string), la cadena de caracteres para convertir.
 * DS:	Retorna un puntero a la primera posición de la cadena de texto en minúsculas.
 * */
char * minusculas(char * str);

 /** verificarPHP:
 * Método para desglosar una ruta seguida de argumentos. Se utiliza para los archivos
 * PHP que reciben parámetros, para poder separar la ruta del archivo de sus argumentos.
 * El método retorna en su primer dato únicamente la ruta del archivo, y en su segundo argumento
 * los parámetros dados, si los hubiera.
 * DE: 	Archivo (string), la ruta del archivo a desglosar.
 * DS:	Archivo (string), la ruta del archivo, separado de sus parametros.
 * 		Argumentos (string), los parametros separados de la ruta del archivo, si existiesen.
 * 		En caso de no existir parámetros, argumentos será NULL.
 * */
void verificarPHP(char ** archivo, char ** argumentos);

/** procesarPHP:
 * Método para atender el pedido a un archivo PHP. Dada la ruta de un archivo y 
 * sus respectivos parámetros, el método se encarga de llamar al CGI de PHP
 * y de responder a través del socket según lo resultante de PHP-CGI.
 * DE: 	Socket (int), el socket asociado al hilo donde se responderá.
 * 		Archivo (string), la ruta del archivo PHP.
 * 		Parametros (string), los parámetros de la ejecución, si existiesen.
 * */
void procesarPHP(int sock, char * archivo, char * parametros);

/** atenderPedido:
 * Método principal. Dado un socket, se encarga de atenderlo.
 * El método se encarga de toda la inteligencia del servidor:
 * - Obtener el mensaje, analizarlo y devolver el Response HTTP según corresponda.
 * DE: 	Socket (int), el socket asociado al hilo donde se harán las lecturas y escrituras.
 * */
void atenderPedido(int sock);

#endif // SERVIDORHTTP_H_
