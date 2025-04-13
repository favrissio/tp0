#include"utils.h"

t_log* logger;

int iniciar_servidor(void)
{
	// Quitar esta línea cuando hayamos terminado de implementar la funcion
	//assert(!"no implementado!");
    int err;
    
	int socket_servidor;

	struct addrinfo hints, *servinfo/*, *p*/;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	err = getaddrinfo(NULL, PUERTO, &hints, &servinfo);

	// Creamos el socket de escucha del servidor
	socket_servidor = socket(servinfo->ai_family,
                        		 servinfo->ai_socktype,
                        		 servinfo->ai_protocol);

	if(socket_servidor == -1){
		 error_show("No se pudo crear socket del servidor");
		 abort();
	}

	// Configuro para que varios socket se puedan bindear al mismo puerto
	err = setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));
	
	// Asociamos el socket a un puerto
	err = bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

	if (err == -1) {
    	error_show("No se pudo hacer bind");
    	abort();
	}

	// Escuchamos las conexiones entrantes
    err = listen(socket_servidor, SOMAXCONN);

	if (err == -1) {
    	error_show("No se pudo hacer listen");
    	abort();
	}

	freeaddrinfo(servinfo);
	log_trace(logger, "Listo para escuchar a mi cliente");

	return socket_servidor;
}

int esperar_cliente(int socket_servidor)
{
	// Quitar esta línea cuando hayamos terminado de implementar la funcion
	//assert(!"no implementado!");

	// Aceptamos un nuevo cliente
	int socket_cliente;
	socket_cliente = accept(socket_servidor, NULL, NULL);
	log_info(logger, "Se conecto un cliente!");

	return socket_cliente;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0){
		return cod_op;
	} else {
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje: %s", buffer);
	free(buffer);
}

t_list* recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}

void handshake_server(int socket_cliente){

	// Variable para llevar conteo de bytes enviados/recibidos
	size_t bytes;
	
	int32_t handshake;
	int32_t resultOk = 0;
	int32_t resultError = -1;

	bytes = recv(socket_cliente, &handshake, sizeof(int32_t), MSG_WAITALL);
	log_info(logger, "bytes recibidos %lu", bytes);
	
	if (handshake == 1) {
    	bytes = send(socket_cliente, &resultOk, sizeof(int32_t), 0);
		log_info(logger, "bytes enviados %lu", bytes);
	} else {
    	bytes = send(socket_cliente, &resultError, sizeof(int32_t), 0);
		log_info(logger, "bytes enviados %lu", bytes);
	}
}
