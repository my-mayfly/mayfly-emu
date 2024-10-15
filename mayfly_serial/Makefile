BUILD_DIR = ./build
NAME = mayfly-serial
BINARY = $(BUILD_DIR)/$(NAME)
CFLAGS = -O2 -MMD -Wall -Werror
LDFLAGS += -lpthread -lSDL2 -ldl -lpcre -lreadline -lrt
OBJS = mayfly_serial.o
$(OBJS):mayfly_serial.c 
	@$(CC) $(CFLAGS) -c $< -o $@
$(BINARY):$(OBJS)
	@mkdir -p $(dir $@)
	g++ -o $@ $< $(LDFLAGS)
run:$(BINARY)
	$(BINARY)
