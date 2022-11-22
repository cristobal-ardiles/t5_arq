=================================================
Esta es la documentación para compilar y ejecutar
=================================================

Se está ejecutando el comando: less README.txt

***************************
*** Para salir: tecla q ***
***************************

Para avanzar a una nueva página: tecla <page down>
Para retroceder a la página anterior: tecla <page up>
Para avanzar una sola línea: tecla <enter>
Para buscar un texto: tecla / seguido del texto (/...texto...)
         por ejemplo: /ddd

-----------------------------------------------

Instrucciones para la tarea 5

Para la parte a, copie el archivo sort-rv-max.s con su solución de la tarea 3
en este directorio.  Compile con este comando:

  make sort-rv-max.ram

Vaya al módulo Ram del circuito lrv32im-t5a.circ.  Seleccione la
ROM y elija Borrar Contenidos.  Cargue el archivo sort-rv-max.ram en
la ROM.  A veces la carga es incompleta si Ud. no borra previamente
los contenidos.  Grabe el circuito y ejecute con control-r y control-k.
El programa debe felicitarlo.  En esta parte Ud. debe entregar (i) el
archivo sort-rv-max.s, (ii) el archivo grabado de lrv32im-t5a.circ,
y (iii) una captura de la ventana completa del circuito mostrando que
se completó la ejecución del programa.

Para la parte b use el circuito lrv32im-t5b.circ incluido en este directorio.
Siga las instrucciones dadas en el enunciado de la tarea 5.

Para la parte c, modifique el archivo ucode-t5c.c incluido en este directorio.
Recuerde que debe usar la señal de control CSRPCTYPE. Compile con:

  make ucode-t5c.rom

Para probar su implementación de swrpc, vaya al módulo Control
Decode Unit del circuito lrv32im-t5c.circ.  Seleccione la ROM con el
microcódigo y elija Borrar Contenidos.  Cargue el archivo ucode-t5c.rom
en la ROM.  Ejecute el programa en assembler pregrabado (test-swrpc.S) con
control-r y control-k.  Lo felicitará si su implementación es correcta.
Si no lo es, vuelva a ejecutar activando la entrada Break del debugger.
La ejecución se detendrá justo en la instrucción swrpc s1,s0,0x70(pc).
Revise cuidadosamente que sus señales de control producen el efecto
deseado.  En esta parte Ud. debe entregar el archivo ucode-t5c.rom
modificado con su implementación de swrpc y una captura de pantalla
mostrando que se completó la ejecución del programa.

-----------------------------------------------

Programas disponibles:
clase.s : ejemplo de ejecución de add, sb y bge
ifact.c : calcula el factorial en formato int
rfact.c : calcula el factorial en formato double, pero se demora una hora
          en mostrar el resultado
fast-rfact.c : como rfact.c pero muestra el resultado mucho más rápido
stdfact.c : como ifact.c pero usa fgets para leer y printf para mostrar,
          pero se demora mucho más que ifact.c.

Algunos programas pueden ejecutarse en Linux con qemu-riscv32 o en
LRV32IM con Logisim.

-----------------------------------------------

Para probar en LRV32IM, por ejemplo clase.s o ifact.c :

make -B clase.ram

(La opción -B hace que make reconstruya todo desde cero, sin importar
que el archivo fuente no haya sido modificado.)

Abrir logisim y cargar lrv32im-up.circ, entrar al módulo RAM, seleccionar
la componente de memoria RAM, abrir menu con botón derecho y seleccionar
la opción cargar imagen.  Se abre un panel que permite seleccionar un archivo.
Navegue hasta encontrar el archivo clase.ram y ábralo.
A continuación regrese al módulo cpu y seleccione el menú Simular -> Activar
reloj (o control-K).  La salida se observa en el Display.  Para la
entrada, seleccione con la mano "Keyboard" e ingrese un texto.
Si el programa está leyendo, aparecerá en el display, si no, quedará
encolado en keyboard a la espera de que el programa lo lea.

Si necesitan volver a ejecutar, deben resetear con Simular -> Resetear
simulación.  Esto borra la RAM.  Deben volver a cargar la imagen del programa
(por ejemplo clase.ram) en la memoria.  Y solo después rejecutar con control-K.

¡El Debugger!

La ejecución se detiene si la entrada Break del módulo Debugger está
en 1 y el Pc (contador de programa) es la dirección ingresada en BrkPoint
o si se escribe en la dirección ingresada en WrWatch.   Para retormar
la ejecución coloque Break en 0.

Determinar qué instrucción se está ejecutando

También puede ejecutar paso a paso las instrucciones con Simular -> Conmutar
reloj (o control-T).  Para saber que instrucción se está ejecutando fíjese
en la dirección marcada abajo en el tunel PC.  Digamos que es 0x10150.
En el terminal ejecute por ejemplo make clase.dump.  Eso usará el comando
less para desensamblar el binario de clase.s, mostrando la dirección de cada
instrucción de máquina.  Busque con /10150 y lo llevará a la instrucción
que se está ejecutando actualmente.

-----------------------------------------------

Para probar los mismos programas con el emulador qemu-riscv32 hay varias
opciones, ejecutando estos comandos en el terminal:

make -B ifact.run

Compila el programa con opciones de optimización y lo ejecuta directamente
en el terminal.

make -B ifact.ddd

Compila el programa con opciones de depuración, lanza ddd para depurar el
programa y lanza el programa de manera que la ejecución se pueda depurar
con ddd.  Defina los breakpoints que necesite y comience la ejecución con
el comando cont en ddd.

Los binarios para qemu recurren a otra implementación de la entrada/salida,
haciendo que funciones como showStr, muestren efectivamente en la salida
estándar de Linux.  Esta implementación está en term-qemu.c.  Para los
binarios que se ejecutarán con LRV32IM, la implementación está en term2.c.

Si necesita reejecutar el programa, porque por ejemplo hizo cambios
en el programa fuente, no vuelva a invocar make -B ifact.ddd
porque perderá los breakpoints.  Mate el programa actualmente en ejecución
ingresando en ddd: kill
Luego invoque en el terminal: make ifact.rerun-g
y en ddd: target remote localhost:1234
y luego: cont

ddd no es perfecto y a veces se cae.  Ahí no queda otra que relanzarlo.
El problema es que a veces el emulador sigue corriendo ocupando
localhost:1234 y no se puede volver a probar otro programa.  Si le ocurre,
ejecute este comando en un terminal:

ps a | grep qemu-riscv32

Si la salida es por ejemplo:

 4294 pts/2    Sl+    0:00 qemu-riscv32 ifact.qemu-O
 4301 pts/3    S+     0:00 grep qemu-riscv32

El proceso que tiene tomado localhost:1234 es el 4294.  Mátelo con:

kill -9 4294

Ahora podrá volver a probar programas con ddd y qemu.

-----------------------------------------------

Ud. puede crear sus propios programas y probarlos con LRV32IM y qemu,
siempre y cuando la entrada y salida se limite a llamar a las funciones
declaradas en term.h.  Por ejemplo si creo miprog.c, tiene todas estas
opciones para probar:

make -B miprog.ram
make -B miprog.run-O
make -B miprog.ddd-g  (y opcionalmente make -B miprog.rerun-g)

-----------------------------------------------

Para ver el assembler generado por gcc para un programa en c:

make ifact.s

(Compila con opciones de optimización)

Luego vea el contenido del archivo ifact.s.

-----------------------------------------------

Para recompilar el programa que genera el u-código con las señales de control:

make -B ucode-up3.rom

Debe cargar el archivo ucode-up3.rom en la ROM ubicada en el módulo
Control Unit (cu2).  Para cargarlo, opere de la misma manera que cargo
el archivo .ram en la memoria RAM.  Como es una ROM, no se pierde su
contenido cuando resetea la simulación, pero si desea preservar su contenido
para la próxima vez que cargue LRV32IM, debe grabar el circuito.

-----------------------------------------------

Para hacer limpieza y borrar todos los archivos generados:

make clean

De todas formas esos archivos se puede reconstruir con: make archivo
