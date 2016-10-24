SOBJS =		serverinterfaceadapter.o textinterface.o interface.o cursesinterface.o cursesdisplays.o servermain.o wlclient.o server.o connection.o input.o netmanager.o netmessage.o streambuffer.o wlconnection.o card.o timer.o
COBJS = 	cursesdisplays.o clientinterfaceadapter.o client.o clientmain.o textinterface.o cursesinterface.o connection.o input.o netmanager.o interface.o netmessage.o streambuffer.o wlconnection.o card.o timer.o	
TOBJS = 	test.o input.o netmessage.o 
GOBJS =		garbage.o input.o netmessage.o netmanager.o connection.o streambuffer.o

ifeq ($(OS),Windows_NT)
	EXEFLAGS =	-g -Wall -Wno-switch
	OBJFLAGS =	-g -Wall -Wno-switch	
	LIBS =	-lwsock32 -lpdcurses
	STARGET =	server.exe
	CTARGET = 	client.exe
	TTARGET =	test.exe
	GTARGET = 	garbage.exe
	RM		=	del
else
	EXEFLAGS =	-g -Wall -Wno-switch
	OBJFLAGS =	-g -Wall -Wno-switch	
	LIBS =	-lcurses -lrt
	STARGET =	server
	CTARGET = 	client
	TTARGET =	test
	GTARGET =   garbage
	RM		=	rm
endif 

all: server client

server: $(SOBJS)
	g++ $(SOBJS) $(EXEFLAGS) -o $(STARGET) $(LIBS)
	
client: $(COBJS)
	g++  $(COBJS) $(EXEFLAGS) -o $(CTARGET) $(LIBS)

test: $(TOBJS)
	g++ $(TOBJS) $(EXEFLAGS) -o $(TTARGET) $(LIBS)
	
garbage: $(GOBJS)
	g++ $(GOBJS) $(EXEFLAGS) -o $(GTARGET) $(LIBS)

%.o: %.cpp
	g++ -c $(OBJFLAGS) $<

clean:
	$(RM) *o $(STARGET) $(CTARGET)
	
