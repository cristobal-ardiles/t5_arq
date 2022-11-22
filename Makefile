RISCV = /opt/riscv
TARGET = riscv64-unknown-elf
CC = ${RISCV}/bin/${TARGET}-gcc -march=rv32im -mabi=ilp32
CXX = ${RISCV}/bin/${TARGET}-g++ -march=rv32im -mabi=ilp32
AS = ${RISCV}/bin/${TARGET}-as
LD = ${RISCV}/bin/${TARGET}-ld
STRIP = ${RISCV}/bin/${TARGET}-strip
GDB = ${RISCV}/bin/${TARGET}-gdb
PK = ${RISCV}/${TARGET}/bin/pk
DUMP= ${RISCV}/${TARGET}/bin/objdump
READELF= ${RISCV}/bin/${TARGET}-readelf
BINCONV2=java BinaryConverter2
QEMU=qemu-riscv32

MAK= make --no-print-directory

CXXFLAGS=-std=c++0x -Wall -pedantic
GXXFLAGS=-g $(CXXFLAGS)

GFLAGS= -g -Wall -pedantic -std=c2x
OFLAGS= -O -Wall -pedantic -std=c2x
CFLAGS= ${OFLAGS}
LDFLAGS=

XTRA=

LRVSRCS=term.c crt0.S $(XTRA)
QEMUSRCS= term-qemu.c $(XTRA)

all:
	less README.txt

sort-rv-max.ram: sort-rv-max.s
	echo "hola"
	${MAK} XTRA=sort.c sort-rv-max.lrv
	${DUMP} -s sort-rv-max.lrv | ${BINCONV2} `${READELF} -a sort-rv-max.lrv | grep " _start"` > $@

%.s: %.c
	${CC} -S ${OFLAGS} $<

%.dump: %.lrv
	${DUMP} -S $(*F).lrv | less

%.ddd-l:
	@${MAK} -B "CFLAGS=$(OFLAGS)" $(*F).lrv
	ddd --debugger ${GDB} --command target.gdb $(*F).lrv &
	@echo "No es muy util tratar de depurar este binario en ddd."
	@echo "Si es util para desensamblar el codigo que se esta ejecutando"
	@echo "en Logisim.  Vea la direccion en el tunel PC (abajo a la"
	@echo "izquierda). Si la direccion es 0x10150 por ejemplo, desensable"
	@echo "con este comando de ddd: x/20i 0x10150"
	@echo "Eso mostrara las siguientes 20 instrucciones que"
	@echo "se van a ejecutar."
	${QEMU} -g 1234 $(*F).lrv

%.rom: %.c
	gcc $(GFLAGS) $(*F).c -o $(*F)
	./$(*F)

%.qemu-O: %.c ${QEMUSRCS}
	${CC} ${OFLAGS} ${LDFLAGS} $^ ${LDLIBS} -o $@

%.qemu-g: %.c ${QEMUSRCS}
	${CC} ${GFLAGS} ${LDFLAGS} $^ ${LDLIBS} -o $@

%.qemu-O: %.s ${QEMUSRCS}
	${CC} ${OFLAGS} ${LDFLAGS} $^ ${LDLIBS} -o $@

%.qemu-g: %.s ${QEMUSRCS}
	${CC} ${GFLAGS} ${LDFLAGS} $^ ${LDLIBS} -o $@

%.run: %.qemu-O
	${QEMU} $<

%.ddd: %.qemu-g
	ddd --debugger ${GDB} --command target.gdb $^ &
	@echo "Coloque los breakpoints que necesite"
	@echo "Para comenzar la ejecucion, ingrese en ddd: cont"
	@echo "Si necesita ejecutar nuevamente, ingrese en ddd: kill"
	@echo "Y ejecute en el terminal: make $(*F).rerun-g"
	@echo "Luego ejecute en ddd: target remote localhost:1234"
	@echo "Y finalmente en ddd: cont"
	${QEMU} -g 1234 $(*F).qemu-g

%.ddd-O: %.qemu-O
	ddd --debugger ${GDB} --command target.gdb $^ &
	@echo "Coloque los breakpoints que necesite"
	@echo "Para comenzar la ejecucion, ingrese en ddd: cont"
	@echo "Si necesita ejecutar nuevamente, ingrese en ddd: kill"
	@echo "Y ejecute en el terminal: make $(*F).rerun-g"
	@echo "Luego ejecute en ddd: target remote localhost:1234"
	@echo "Y finalmente en ddd: cont"
	${QEMU} -g 1234 $(*F).qemu-O

%.rerun-g: %.qemu-g
	@echo "Ejecute en ddd: target remote localhost:1234"
	@echo "Y luego: cont"
	${QEMU} -g 1234 $(*F).qemu-g

%.lrv: %.c ${LRVSRCS}
	${CC} ${OFLAGS} ${LDFLAGS} -nostartfiles $^ ${LDLIBS} -o $@

%.lrv: %.S ${LRVSRCS}
	${CC} ${OFLAGS} ${LDFLAGS} -nostartfiles $^ ${LDLIBS} -o $@

%.lrv: %.s ${LRVSRCS}
	${CC} ${OFLAGS} ${LDFLAGS} -nostartfiles $^ ${LDLIBS} -o $@

%.ram: %.lrv
	${DUMP} -s $< | ${BINCONV2} `${READELF} -a $< | grep " _start"` > $@

clean:
	rm -f *.o *.ram ucode-up *.lrv *.qemu-O *.qemu-g
