all:MirrorInitiator MirrorServer ContentServer

MirrorInitiator:main_mirror_initiator.o
	gcc -pthread -o MirrorInitiator main_mirror_initiator.o -g 

main_mirror_initiator.o:main_mirror_initiator.c 
	gcc -pthread -g -c main_mirror_initiator.c

MirrorServer:main_mirror_server.o
	gcc -pthread -o MirrorServer main_mirror_server.o -g

main_mirror_server.o:main_mirror_server.c
	gcc -pthread  -g -c main_mirror_server.c

ContentServer:main_contentserver.o
	gcc -pthread -o ContentServer main_contentserver.o -g

main_contentserver.o:main_contentserver.c
	gcc -pthread -g -c main_contentserver.c
