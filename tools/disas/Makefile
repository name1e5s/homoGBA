# Makefile is easier than cmake.

CC:= gcc
NAME:= disas
MAJOR:= 1
MINOR:= 0

BINUTILS = binutils-2.27
BFD = $(BINUTILS)/bfd/
OPCODES = $(BINUTILS)/opcodes/
IBERTY = $(BINUTILS)/libiberty/
BFD_LIB = $(BINUTILS)/bfd/libbfd.a
OPCODES_LIB = $(BINUTILS)/opcodes/libopcodes.a
IBERTY_LIB = $(BINUTILS)/libiberty/libiberty.a
TARGETS = "arm-linux-gnu armel-linux-gnu mipsel-linux-gnu"
CFLAGS := -I$(BINUTILS)/include -I$(BFD) -O3 -Wall -Werror -DPACKAGE

lib: binutils lib$(NAME).a

binutils:
	test -d $(BINUTILS) || (wget https://mirrors.tuna.tsinghua.edu.cn/gnu/binutils/$(BINUTILS).tar.gz && tar -zxf $(BINUTILS).tar.gz)
	test -e $(BFD_LIB) || (cd $(BFD) && ./configure --enable-targets=$(TARGETS) && make -j 4 LDFLAGS="-all-static")
	test -e $(OPCODES_LIB) || (cd $(OPCODES) && ./configure --enable-targets=$(TARGETS) && make -j 4 LDFLAGS="-all-static")
	test -e $(IBERTY_LIB) || (cd $(IBERTY) && ./configure && make -j 4 LDFLAGS="-all-static")
	rm -rf $(BINUTILS).tar.gz
	
libtmp$(NAME).a:
	$(CC) $(CFLAGS) -c $(NAME).c
	ar -rc libtmp$(NAME).a $(NAME).o
	
lib$(NAME).a: libtmp$(NAME).a binutils
	ar -M < link.ars
	
clean:
	$(RM) *.o *.a *tmp*
	$(RM) -rf $(BINUTILS)
	$(RM) -rf $(BINUTILS).tar.gz
