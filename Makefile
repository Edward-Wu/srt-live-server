SHELL = /bin/sh
MAIN_NAME=sls
CLIENT_NAME=sls_client
INC_PATH = -I./ -I../ -I./slscore -I./include
LIB_PATH =  -L ./lib
LIBRARY_FILE = -lpthread -lz -lsrt
BIN_PATH = ./bin

DEBUG = -g
CFLAGS += $(DEBUG)

LOG_PATH = ./logs


OUTPUT_PATH = ./obj
OBJS = $(OUTPUT_PATH)/SLSLog.o \
	$(OUTPUT_PATH)/common.o\
	$(OUTPUT_PATH)/conf.o\
	$(OUTPUT_PATH)/SLSThread.o\
	$(OUTPUT_PATH)/SLSEpollThread.o\
	$(OUTPUT_PATH)/SLSManager.o\
	$(OUTPUT_PATH)/SLSGroup.o\
	$(OUTPUT_PATH)/SLSRole.o\
	$(OUTPUT_PATH)/SLSListener.o\
	$(OUTPUT_PATH)/SLSRoleList.o\
	$(OUTPUT_PATH)/SLSSrt.o\
	$(OUTPUT_PATH)/SLSPublisher.o\
	$(OUTPUT_PATH)/SLSPlayer.o\
	$(OUTPUT_PATH)/SLSRecycleArray.o\
	$(OUTPUT_PATH)/SLSMapData.o\
	$(OUTPUT_PATH)/SLSMapPublisher.o\
	$(OUTPUT_PATH)/SLSRelay.o\
	$(OUTPUT_PATH)/SLSPuller.o\
	$(OUTPUT_PATH)/SLSPusher.o\
	$(OUTPUT_PATH)/SLSRelayManager.o\
	$(OUTPUT_PATH)/SLSPullerManager.o\
	$(OUTPUT_PATH)/SLSPusherManager.o\
	$(OUTPUT_PATH)/SLSMapRelay.o\
	$(OUTPUT_PATH)/SLSClient.o\
	$(OUTPUT_PATH)/TCPRole.o\
	$(OUTPUT_PATH)/SLSArray.o\
	$(OUTPUT_PATH)/HttpRoleList.o\
	$(OUTPUT_PATH)/HttpClient.o			
	
CORE_PATH = slscore
COMMON_FILES = common.hpp

all: $(OBJS)
	mkdir -p ${LOG_PATH}
	mkdir -p ${OUTPUT_PATH}
	mkdir -p ${BIN_PATH}
	g++ -o ${BIN_PATH}/${MAIN_NAME}   srt-live-server.cpp $(OBJS) $(CFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBRARY_FILE)
	g++ -o ${BIN_PATH}/${CLIENT_NAME} srt-live-client.cpp $(OBJS) $(CFLAGS) $(INC_PATH) $(LIB_PATH) $(LIBRARY_FILE)
	#******************************************************************************#
	#                          Bulid successful !                                  #
	#******************************************************************************#

$(OUTPUT_PATH)/common.o: ./$(CORE_PATH)/common.cpp
	g++ -c $(CFLAGS) $< -o $@ $(INC_FLAGS)

$(OUTPUT_PATH)/conf.o: ./$(CORE_PATH)/conf.cpp
	g++ -c $(CFLAGS) $< -o $@ $(INC_FLAGS)

$(OUTPUT_PATH)/SLSLog.o: ./$(CORE_PATH)/SLSLog.cpp
	g++ -c $(CFLAGS) $< -o $@ $(INC_FLAGS)
	
$(OUTPUT_PATH)/SLSThread.o: ./$(CORE_PATH)/SLSThread.cpp 
	g++ -c $(CFLAGS) $< -o $@ $(INC_PATH)
	
$(OUTPUT_PATH)/SLSEpollThread.o: ./$(CORE_PATH)/SLSEpollThread.cpp 
	g++ -c $(CFLAGS) $< -o $@ $(INC_PATH)
	
$(OUTPUT_PATH)/SLSManager.o: ./$(CORE_PATH)/SLSManager.cpp 
	g++ -c $(CFLAGS) $< -o $@ $(INC_PATH)
	
$(OUTPUT_PATH)/SLSGroup.o: ./$(CORE_PATH)/SLSGroup.cpp 
	g++ -c $(CFLAGS) $< -o $@ $(INC_PATH)
	
$(OUTPUT_PATH)/SLSRole.o: ./$(CORE_PATH)/SLSRole.cpp 
	g++ -c $(CFLAGS) $< -o $@ $(INC_FLAGS)
	
$(OUTPUT_PATH)/SLSListener.o: ./$(CORE_PATH)/SLSListener.cpp 
	g++ -c $(CFLAGS) $< -o $@ $(INC_PATH)
	
$(OUTPUT_PATH)/SLSRoleList.o: ./$(CORE_PATH)/SLSRoleList.cpp 
	g++ -c $(CFLAGS) $< -o $@ $(INC_PATH)
	
$(OUTPUT_PATH)/SLSSrt.o: ./$(CORE_PATH)/SLSSrt.cpp 
	g++ -c $(CFLAGS) $< -o $@ $(INC_PATH)
	
$(OUTPUT_PATH)/SLSPublisher.o: ./$(CORE_PATH)/SLSPublisher.cpp 
	g++ -c $(CFLAGS) $< -o $@ $(INC_PATH)
	
$(OUTPUT_PATH)/SLSPlayer.o: ./$(CORE_PATH)/SLSPlayer.cpp 
	g++ -c $(CFLAGS) $< -o $@ $(INC_PATH)
	
$(OUTPUT_PATH)/SLSRecycleArray.o: ./$(CORE_PATH)/SLSRecycleArray.cpp 
	g++ -c $(CFLAGS) $< -o $@ $(INC_PATH)
	
$(OUTPUT_PATH)/SLSMapData.o: ./$(CORE_PATH)/SLSMapData.cpp 
	g++ -c $(CFLAGS) $< -o $@ $(INC_PATH)

$(OUTPUT_PATH)/SLSMapPublisher.o: ./$(CORE_PATH)/SLSMapPublisher.cpp 
	g++ -c $(CFLAGS) $< -o $@ $(INC_PATH)

$(OUTPUT_PATH)/SLSRelay.o: ./$(CORE_PATH)/SLSRelay.cpp 
	g++ -c $(CFLAGS) $< -o $@ $(INC_PATH)

$(OUTPUT_PATH)/SLSPuller.o: ./$(CORE_PATH)/SLSPuller.cpp 
	g++ -c $(CFLAGS) $< -o $@ $(INC_PATH)

$(OUTPUT_PATH)/SLSPusher.o: ./$(CORE_PATH)/SLSPusher.cpp 
	g++ -c $(CFLAGS) $< -o $@ $(INC_PATH)

$(OUTPUT_PATH)/SLSRelayManager.o: ./$(CORE_PATH)/SLSRelayManager.cpp 
	g++ -c $(CFLAGS) $< -o $@ $(INC_PATH)

$(OUTPUT_PATH)/SLSPullerManager.o: ./$(CORE_PATH)/SLSPullerManager.cpp 
	g++ -c $(CFLAGS) $< -o $@ $(INC_PATH)

$(OUTPUT_PATH)/SLSPusherManager.o: ./$(CORE_PATH)/SLSPusherManager.cpp 
	g++ -c $(CFLAGS) $< -o $@ $(INC_PATH)

$(OUTPUT_PATH)/SLSMapRelay.o: ./$(CORE_PATH)/SLSMapRelay.cpp 
	g++ -c $(CFLAGS) $< -o $@ $(INC_PATH)

$(OUTPUT_PATH)/SLSClient.o: ./$(CORE_PATH)/SLSClient.cpp 
	g++ -c $(CFLAGS) $< -o $@ $(INC_PATH)

$(OUTPUT_PATH)/TCPRole.o: ./$(CORE_PATH)/TCPRole.cpp 
	g++ -c $(CFLAGS) $< -o $@ $(INC_PATH)

$(OUTPUT_PATH)/SLSArray.o: ./$(CORE_PATH)/SLSArray.cpp 
	g++ -c $(CFLAGS) $< -o $@ $(INC_PATH)

$(OUTPUT_PATH)/HttpRoleList.o: ./$(CORE_PATH)/HttpRoleList.cpp 
	g++ -c $(CFLAGS) $< -o $@ $(INC_PATH)

$(OUTPUT_PATH)/HttpClient.o: ./$(CORE_PATH)/HttpClient.cpp 
	g++ -c $(CFLAGS) $< -o $@ $(INC_PATH)

clean:
	rm -f $(OUTPUT_PATH)/*.o
	rm -rf $(BIN_PATH)/*

