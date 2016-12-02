# Mini Servidor HTTP

# Compilacion

Para compilar el proyecto usar GNU GCC. En terminal escribir

```
gcc servidorHTTP.c -o servidorHTTP
```

# Modo de uso

Para levantar un servidor, correr el siguiente comando

```
./servidorHTTP [IP][:puerto] [-h]
```

En caso de no especificarse IP o puerto se utilizará por defecto 127.0.0.1:80

