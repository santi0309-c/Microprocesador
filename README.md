# Casos de Prueba — Instrucciones Tipo R (1 a 32)
### RTM32 / STX4 — Testing de instruction set

# Caso 1

## Descripción
Testeo de `SLL` (Shift Left Logical), shift a la izquierda usando una constante inmediata (`aux`, 0-31) embebida en la instrucción.

## Instrucciones
`SLL`

## Precondiciones
- $t0 ($2) = 0x00000001

## Code
```asm
sll $t1, $t0, 4      # R[$t1] = R[$t0] << 4
```

## Postcondiciones
- Verificar $t1 ($3) mediante `examine /xw $3 1` (o equivalente)
- Valor esperado: $t1 = 0x00000010

## Conclusiones
Anduvo. 0x1 desplazado 4 bits a la izquierda da 0x10 (16 decimal), coincide con el valor observado en $t1. El campo `aux` de 5 bits permite shifts de 0 a 31 posiciones.

---

# Caso 2

## Descripción
Testeo de `SRL` (Shift Right Logical), shift lógico a la derecha (relleno con ceros, sin importar el signo).

## Instrucciones
`SRL`

## Precondiciones
- $t0 ($2) = 0xF0000000

## Code
```asm
srl $t1, $t0, 4      # R[$t1] = R[$t0] >> 4 (logico)
```

## Postcondiciones
- Verificar $t1 ($3)
- Valor esperado: $t1 = 0x0F000000

## Conclusiones
Anduvo. Al ser un shift lógico, el bit de signo (MSB=1) no se preserva y se rellena con ceros: 0xF0000000 >> 4 = 0x0F000000. Esto confirma que `SRL` no hace sign extension, a diferencia de `SRA`.

---

# Caso 3

## Descripción
Testeo de `SRA` (Shift Right Arithmetic), shift aritmético a la derecha (preserva el signo, relleno con el bit MSB).

## Instrucciones
`SRA`

## Precondiciones
- $t0 ($2) = 0xF0000000 (número negativo en complemento a 2)

## Code
```asm
sra $t1, $t0, 4      # R[$t1] = R[$t0] >>> 4 (aritmetico)
```

## Postcondiciones
- Verificar $t1 ($3)
- Valor esperado: $t1 = 0xFF000000

## Conclusiones
Anduvo. Con el mismo valor de entrada que el Caso 2 pero usando `SRA` en vez de `SRL`, el resultado difiere: se preserva el bit de signo (1) en las posiciones que quedan libres, dando 0xFF000000 en vez de 0x0F000000. Este contraste con el Caso 2 es la mejor forma de confirmar que ambas instrucciones están bien diferenciadas en la máquina.

---

# Caso 4

## Descripción
Testeo de `SLLR` (Shift Left Logical Register), variante de `SLL` donde el monto de desplazamiento no es una constante sino que viene de un registro (bits [4:0]).

## Instrucciones
`SLLR`

## Precondiciones
- $t0 ($2) = 0x00000001
- $t1 ($3) = 0x00000003

## Code
```asm
sllr $t2, $t0, $t1   # R[$t2] = R[$t0] << R[$t1][4:0]
```

## Postcondiciones
- Verificar $t2 ($4)
- Valor esperado: $t2 = 0x00000008

## Conclusiones
Anduvo. 1 desplazado 3 posiciones a la izquierda da 8, coincide con lo observado. Importante: solo se toman los 5 bits menos significativos del registro que indica el monto de shift (R[rs][4:0]), por lo que valores de shift mayores a 31 en $t1 deberían dar el mismo resultado que su módulo 32 — pendiente de testear como caso adicional si se quiere confirmar el wraparound.

---

# Caso 5

## Descripción
Testeo de `SRLR` (Shift Right Logical Register), análogo lógico de `SLLR` pero desplazando a la derecha.

## Instrucciones
`SRLR`

## Precondiciones
- $t0 ($2) = 0x000000F0
- $t1 ($3) = 0x00000004

## Code
```asm
srlr $t2, $t0, $t1   # R[$t2] = R[$t0] >> R[$t1][4:0]  (logico)
```

## Postcondiciones
- Verificar $t2 ($4)
- Valor esperado: $t2 = 0x0000000F

## Conclusiones
Anduvo. 0xF0 desplazado 4 bits a la derecha (lógico) da 0x0F, tal como se observó. Al no tener bit de signo activado en la entrada (MSB=0) no se puede distinguir todavía de un shift aritmético con este caso; para eso se usa el Caso 6.

---

# Caso 6

## Descripción
Testeo de `SRAR` (Shift Right Arithmetic Register), análogo aritmético de `SRLR`, usando un valor negativo para confirmar la extensión de signo.

## Instrucciones
`SRAR`

## Precondiciones
- $t0 ($2) = 0x80000000 (negativo, MSB=1)
- $t1 ($3) = 0x00000004

## Code
```asm
srar $t2, $t0, $t1   # R[$t2] = R[$t0] >>> R[$t1][4:0]  (aritmetico)
```

## Postcondiciones
- Verificar $t2 ($4)
- Valor esperado: $t2 = 0xF8000000

## Conclusiones
Anduvo. Se preserva el bit de signo (1) en las 4 posiciones desplazadas, dando 0xF8000000. Combinado con el Caso 5 (mismo mecanismo pero con `SRLR`, resultado sin signo), queda confirmado que la diferenciación lógica/aritmética funciona igual para la familia "R" (shift por registro) que para la familia con `aux` constante (Casos 1-3).

---

# Caso 7

## Descripción
Testeo de `CFS` (Copy From Special), que copia el contenido de un registro especial (`S[aux]`) hacia un registro de propósito general. Se aprovecha que `$vbr` tiene un valor conocido de fábrica tras el reset del procesador (0xF0000000, según el manual), para no depender de haber seteado nada de antemano.

## Instrucciones
`CFS`

## Precondiciones
- Ninguna — se ejecuta apenas se resetea la máquina, sin modificar registros especiales previamente.
- (Asumido) índice de `$vbr` en la tabla de registros especiales = 4, según el orden en que se listan en el manual: psw=0, ecr=1, epc=2, bva=3, vbr=4. **A confirmar contra el assembler real antes de dar el caso por válido**, ya que el manual no publica explícitamente esta tabla de índices.

## Code
```asm
cfs $t0, 4      # R[$t0] = S[4]  (se espera que S[4] = $vbr)
```

## Postcondiciones
- Verificar $t0 ($2)
- Valor esperado: $t0 = 0xF0000000

## Conclusiones
[Completar tras probar en la máquina] — Si el resultado da 0xF0000000, confirma tanto que `CFS` funciona como que el índice asumido para `$vbr` es correcto. Si da otro valor, hay que revisar el índice de `aux` usado, ya que el mecanismo de la instrucción en sí podría seguir siendo correcto pero apuntando a otro registro especial.

---

# Caso 8

## Descripción
Testeo de `CTS` (Copy To Special), que escribe un registro de propósito general hacia un registro especial (`S[aux]`). Se usa `$epc` (Exception Program Counter) en vez de `$psw`, porque `$psw` es un registro de banderas/estado con bits reservados que podrían no aceptar escritura arbitraria, mientras que `$epc` es un registro de propósito más simple (una dirección) y por lo tanto más confiable para un test mecánico de ida y vuelta.

## Instrucciones
`CTS`, `CFS` (se usa `CFS` únicamente para leer de vuelta y verificar, no es el foco del caso)

## Precondiciones
- $t0 ($2) = 0xDEADBEEF
- (Asumido) índice de `$epc` = 2, mismo criterio que en el Caso 7

## Code
```asm
cts $t0, 2      # S[2] = R[$t0]      (se espera que S[2] = $epc)
cfs $t1, 2      # R[$t1] = S[2]      (lectura de verificación)
```

## Postcondiciones
- Verificar $t1 ($3)
- Valor esperado: $t1 = 0xDEADBEEF (igual a $t0)

## Conclusiones
[Completar tras probar en la máquina] — Si $t1 == $t0, confirma que la escritura y lectura del registro especial funcionan de punta a punta. Este test depende del Caso 7 para validar el índice usado; si el índice de `$epc` no es 2, hay que ajustar ambos casos en conjunto.

---

# Caso 9

## Descripción
Testeo de `AND`, operación lógica bit a bit entre dos registros.

## Instrucciones
`AND`

## Precondiciones
- $t0 ($2) = 0x0F0F0F0F
- $t1 ($3) = 0x00FF00FF

## Code
```asm
and $t2, $t0, $t1   # R[$t2] = R[$t0] & R[$t1]
```

## Postcondiciones
- Verificar $t2 ($4)
- Valor esperado: $t2 = 0x000F000F

## Conclusiones
Anduvo. Se eligieron patrones de nibbles alternados para que el AND bit a bit sea fácil de verificar a ojo: solo sobreviven los bits donde ambos operandos tienen 1, dando 0x000F000F. El mismo par de operandos se reutiliza en el Caso 10 para poder comparar directamente AND vs OR sobre los mismos datos.

---

# Caso 10

## Descripción
Testeo de `OR`, operación lógica bit a bit entre dos registros.

## Instrucciones
`OR`

## Precondiciones
- $t0 ($2) = 0x0F0F0F0F
- $t1 ($3) = 0x00FF00FF

## Code
```asm
or $t2, $t0, $t1    # R[$t2] = R[$t0] | R[$t1]
```

## Postcondiciones
- Verificar $t2 ($4)
- Valor esperado: $t2 = 0x0FFF0FFF

## Conclusiones
Anduvo. Usando los mismos operandos que el Caso 9, el resultado de OR (0x0FFF0FFF) es distinto y complementario al de AND (0x000F000F): donde AND deja solo la intersección de bits en 1, OR deja la unión. Comparar ambos casos con el mismo input es una forma rápida de descartar que la máquina esté ejecutando la operación equivocada (por ejemplo, confundir el func code de AND con el de OR).

---

# Caso 11

## Descripción
Testeo de `XOR`, operación lógica bit a bit "or exclusivo" entre dos registros. Se reutilizan los mismos operandos que en los Casos 9 y 10 (`AND`/`OR`) para poder comparar los tres resultados sobre el mismo par de valores y confirmar que la máquina no está confundiendo los func codes de las tres instrucciones lógicas.

## Instrucciones
`XOR`

## Precondiciones
- $t0 ($2) = 0x0F0F0F0F
- $t1 ($3) = 0x00FF00FF

## Code
```asm
xor $t2, $t0, $t1   # R[$t2] = R[$t0] ^ R[$t1]
```

## Postcondiciones
- Verificar $t2 ($4)
- Valor esperado: $t2 = 0x0FF00FF0

## Conclusiones
Anduvo. Comparando los tres resultados sobre el mismo par de operandos: AND=0x000F000F, OR=0x0FFF0FFF, XOR=0x0FF00FF0. Los tres son distintos entre sí y consistentes con el álgebra booleana esperada (XOR = OR sin los bits que coinciden en ambos operandos), lo que da bastante confianza de que las tres instrucciones lógicas están bien implementadas y diferenciadas.

---

# Caso 12

## Descripción
Testeo de `NOR`, negación lógica del OR bit a bit. Se reutiliza el resultado del Caso 10 (`OR` = 0x0FFF0FFF) como referencia directa: `NOR` sobre los mismos operandos debería dar exactamente el complemento a 1 de ese valor.

## Instrucciones
`NOR`

## Precondiciones
- $t0 ($2) = 0x0F0F0F0F
- $t1 ($3) = 0x00FF00FF

## Code
```asm
nor $t2, $t0, $t1   # R[$t2] = ~(R[$t0] | R[$t1])
```

## Postcondiciones
- Verificar $t2 ($4)
- Valor esperado: $t2 = 0xF0000F00

## Conclusiones
Anduvo. 0xF0000F00 es exactamente el complemento a uno de 0x0FFF0FFF (resultado del OR en el Caso 10), tal como predice la definición de NOR. Es un buen caso porque no depende de calcular nada nuevo: si el Caso 10 ya está validado, este caso se verifica por simple inspección (complementar cada dígito hex).

---

# Caso 13

## Descripción
Testeo de `SLT` (Set on Less Than, con signo), comparando un valor negativo contra uno positivo para forzar un caso donde la interpretación con signo importa.

## Instrucciones
`SLT`

## Precondiciones
- $t0 ($2) = 0xFFFFFFFF (interpretado con signo: −1)
- $t1 ($3) = 0x00000001 (1)

## Code
```asm
slt $t2, $t0, $t1   # R[$t2] = (R[$t0] < R[$t1]) con signo ? 1 : 0
```

## Postcondiciones
- Verificar $t2 ($4)
- Valor esperado: $t2 = 0x00000001

## Conclusiones
Anduvo. −1 < 1 en aritmética con signo, por lo que $t2 dio 1 tal como se esperaba. Este mismo par de operandos se reutiliza en el Caso 14 con `SLTU` para forzar el contraste con y sin signo sobre los mismos bits.

---

# Caso 14

## Descripción
Testeo de `SLTU` (Set on Less Than, sin signo), usando exactamente los mismos operandos que el Caso 13 para verificar que la interpretación sin signo da un resultado distinto — 0xFFFFFFFF sin signo es el número más grande representable en 32 bits, así que no puede ser menor que 1.

## Instrucciones
`SLTU`

## Precondiciones
- $t0 ($2) = 0xFFFFFFFF (interpretado sin signo: 4294967295)
- $t1 ($3) = 0x00000001 (1)

## Code
```asm
sltu $t2, $t0, $t1  # R[$t2] = (R[$t0] < R[$t1]) sin signo ? 1 : 0
```

## Postcondiciones
- Verificar $t2 ($4)
- Valor esperado: $t2 = 0x00000000

## Conclusiones
Anduvo. Con los mismos bits que el Caso 13, `SLTU` dio 0 en vez de 1, confirmando que la CPU efectivamente distingue la comparación con signo de la comparación sin signo y no está simplemente ignorando el func bit que las diferencia (igual que ocurre en el `mul/mulh/mulhu` o `div/divu` de la tabla, que comparten el mismo patrón con/sin signo).

---

# Caso 15

## Descripción
Testeo de `JR` (Jump Register), salto incondicional a la dirección contenida en un registro. Para no depender de conocer de antemano el mapa de memoria del programa, la estrategia es: pausar en el debugger, anotar el valor actual de `$pc`, calcular a mano la dirección de una instrucción algunas posiciones más adelante (una especie de "marcador" fácil de reconocer, por ejemplo un `nop` o `addi $zero,$zero,0`), cargarla en un registro, y saltar ahí.

## Instrucciones
`JR`

## Precondiciones
- Con el programa detenido en el debugger, leer el valor de `$pc` (ej: supongamos que da 0x00001000).
- Calcular la dirección target sumando el offset conocido hasta el marcador elegido (ej: si el marcador está 5 instrucciones después, target = 0x00001000 + 5*4 = 0x00001014).
- Cargar esa dirección en un registro:
```asm
lui  $t0, 0x0000
ori  $t0, $t0, 0x1014   # $t0 = target (ajustar según el PC real leído)
```

## Code
```asm
jr $t0        # PC = R[$t0]
```

## Postcondiciones
- Tras ejecutar (step), leer `$pc` en el debugger.
- Valor esperado: $pc == target calculado (0x00001014 en el ejemplo)
- Confirmar además que la siguiente instrucción mostrada por el debugger es efectivamente el marcador elegido, y no la instrucción que seguía secuencialmente antes del salto.

## Conclusiones
[Completar tras probar en la máquina] — Este caso depende de calcular bien la dirección target a partir del `$pc` real que muestre el debugger al momento de la prueba, por eso no se puede dar un valor numérico fijo de antemano como en los casos anteriores. Si el `$pc` resultante coincide con el target y la instrucción siguiente es el marcador, `JR` funciona correctamente.

---

# Caso 16

## Descripción
Testeo de `JALR` (Jump And Link Register), que además de saltar guarda la dirección de retorno. Ojo: la tabla A.2 lista `JALR rs rt` en la codificación pero la fórmula de operación solo menciona `R[31] = PC + 4; PC = R[rs]` — no queda claro en el manual si el campo `rt` se usa realmente como registro de destino configurable para la dirección de retorno, o si siempre escribe en `$31` ($ra) sin importar qué se ponga en `rt`. Este caso está pensado justamente para despejar esa duda.

## Instrucciones
`JALR`

## Precondiciones
- Igual que en el Caso 15: anotar `$pc` actual en el debugger (ej: 0x00002000) y cargar en un registro la dirección target de un marcador conocido (ej: 0x00002020):
```asm
lui  $t0, 0x0000
ori  $t0, $t0, 0x2020   # $t0 = target (ajustar según el PC real leído)
```

## Code
```asm
jalr $t0        # (según formato de la tabla) R[31] = PC + 4 ; PC = R[$t0]
```

## Postcondiciones
- Verificar `$pc` == target (0x00002020 en el ejemplo), igual que en el Caso 15.
- Verificar `$ra` ($31): valor esperado = dirección de la instrucción `jalr` + 4 (ej: si `jalr` estaba en 0x00001FF8, $ra debería quedar en 0x00001FFC).
- Si el assembler soporta pasarle un segundo operando (ej `jalr $rd, $t0`), repetir el caso probando si la dirección de retorno aparece en `$rd` en vez de (o además de) `$31`, para despejar la ambigüedad del campo `rt` mencionada arriba.

## Conclusiones
[Completar tras probar en la máquina] — Si `$ra` queda con PC+4 y `$pc` salta correctamente al target, la instrucción básica funciona. Falta confirmar el rol real del campo `rt`/segundo operando; si al variarlo cambia dónde se guarda la dirección de retorno, hay que corregir la descripción de la tabla A.2 (o reportarlo como posible bug/imprecisión de documentación, similar al bug de `LHU` que encontró Benicio).

---

# Caso 17

## Descripción
Testeo de `LHX` (Load Halfword indeXed, con signo), que calcula la dirección efectiva sumando dos registros (`R[rs]+R[rd]`) en vez de un registro + constante como en `LH`. Se precarga la memoria con una media palabra negativa para verificar la extensión de signo.

## Instrucciones
`LHX`, `SH` (usada solo como setup para escribir el valor conocido en memoria)

## Precondiciones
- $s0 = 0x00002000 (dirección base válida de datos)
- $s1 = 0x00000004 (offset de índice)
- $t0 = 0x00008001 (valor a precargar; su mitad baja 0x8001 tiene el bit de signo del halfword en 1)
```asm
sh $s0, $t0, 4    # M[$s0+4][15:0] = 0x8001   (precarga de memoria, EA = 0x2004)
```

## Code
```asm
lhx $t1, $s0, $s1   # R[$t1] = SignExtend( M[R[$s0]+R[$s1]][15:0] )   EA = 0x2004
```

## Postcondiciones
- Verificar $t1 mediante `examine /xw` sobre el registro
- Valor esperado: $t1 = 0xFFFF8001 (extensión de signo de 0x8001)

## Conclusiones
[Completar tras probar en la máquina] — Si $t1 da 0xFFFF8001, confirma tanto el direccionamiento indexado (base+índice, EA=0x2004) como la extensión de signo. Se recomienda correr este caso junto con el Caso 18 (`LHUX`) sobre la misma dirección de memoria para contrastar signed vs unsigned sin tener que volver a precargar el dato.

---

# Caso 18

## Descripción
Testeo de `LHUX` (Load Halfword Unsigned indeXed), variante sin signo de `LHX`. Se reutiliza la misma dirección de memoria precargada en el Caso 17 para que el único cambio entre ambos casos sea la instrucción en sí, aislando así la diferencia de comportamiento.

## Instrucciones
`LHUX`

## Precondiciones
- Continuación directa del Caso 17: memoria en 0x2004 ya contiene 0x8001 (asumiendo que se corre inmediatamente después, sin resetear la máquina)
- $s0 = 0x00002000, $s1 = 0x00000004 (mismos registros de dirección que el Caso 17)

## Code
```asm
lhux $t1, $s0, $s1   # R[$t1] = ZeroExtend( M[R[$s0]+R[$s1]][15:0] )   EA = 0x2004
```

## Postcondiciones
- Verificar $t1
- Valor esperado: $t1 = 0x00008001 (extensión con ceros, no con signo)

## Conclusiones
[Completar tras probar en la máquina] — Comparando contra el Caso 17 (mismo dato en memoria, mismo direccionamiento): si `LHX` dio 0xFFFF8001 y `LHUX` da 0x00008001, queda confirmado que la diferencia signed/unsigned funciona igual que en los shifts (Casos 2-3 y 5-6) y en `SLT`/`SLTU` (Casos 13-14) — un patrón que se repite en varios pares de instrucciones de esta ISA.

---

# Caso 19

## Descripción
Testeo de `LBX` (Load Byte indeXed, con signo), análogo a `LHX` pero para un byte en vez de un halfword.

## Instrucciones
`LBX`, `SB` (setup)

## Precondiciones
- $s0 = 0x00002000 (dirección base)
- $s1 = 0x00000008 (offset de índice, distinto al usado en los Casos 17/18 para no pisar esos datos)
- $t0 = 0x00000080 (byte negativo: bit 7 = 1)
```asm
sb $s0, $t0, 8    # M[$s0+8][7:0] = 0x80   (precarga, EA = 0x2008)
```

## Code
```asm
lbx $t1, $s0, $s1   # R[$t1] = SignExtend( M[R[$s0]+R[$s1]][7:0] )   EA = 0x2008
```

## Postcondiciones
- Verificar $t1
- Valor esperado: $t1 = 0xFFFFFF80 (extensión de signo de 0x80)

## Conclusiones
[Completar tras probar en la máquina] — Mismo criterio que el Caso 17 pero para bytes. Se recomienda correr junto con el Caso 20 (`LBUX`) sobre la misma dirección para el contraste directo signed vs unsigned.

---

# Caso 20

## Descripción
Testeo de `LBUX` (Load Byte Unsigned indeXed), variante sin signo de `LBX`, reutilizando el mismo byte precargado en el Caso 19.

## Instrucciones
`LBUX`

## Precondiciones
- Continuación directa del Caso 19: memoria en 0x2008 ya contiene 0x80
- $s0 = 0x00002000, $s1 = 0x00000008

## Code
```asm
lbux $t1, $s0, $s1   # R[$t1] = ZeroExtend( M[R[$s0]+R[$s1]][7:0] )   EA = 0x2008
```

## Postcondiciones
- Verificar $t1
- Valor esperado: $t1 = 0x00000080

## Conclusiones
[Completar tras probar en la máquina] — Igual que en el par Caso 17/18, comparar contra el Caso 19: si `LBX` dio 0xFFFFFF80 y `LBUX` da 0x00000080 sobre el mismo byte, confirma la extensión con/sin signo también para el caso de bytes.

---

# Caso 21

## Descripción
Testeo de `LWX` (Load Word indeXed) — corregido de "LWC" del pedido original, ya que esa instrucción no existe en la tabla A.2. Al ser una palabra completa (32 bits) no hay extensión de signo que testear; el foco acá es únicamente confirmar que el direccionamiento indexado (`R[rs]+R[rd]`) calcula bien la dirección efectiva.

## Instrucciones
`LWX`, `SW` (setup)

## Precondiciones
- $s0 = 0x00002000 (dirección base)
- $s1 = 0x0000000C (offset de índice, EA = 0x200C)
- $t0 = 0xCAFEBABE (palabra de 32 bits fácil de reconocer)
```asm
sw $s0, $t0, 12   # M[$s0+12] = 0xCAFEBABE   (precarga, EA = 0x200C)
```

## Code
```asm
lwx $t1, $s0, $s1   # R[$t1] = M[R[$s0]+R[$s1]]   EA = 0x200C
```

## Postcondiciones
- Verificar $t1
- Valor esperado: $t1 = 0xCAFEBABE

## Conclusiones
[Completar tras probar en la máquina] — Si $t1 recupera exactamente el mismo patrón 0xCAFEBABE que se escribió con `SW`, confirma que `LWX` calcula bien la EA indexada y que el par de escritura/lectura de memoria funciona de punta a punta para palabras completas. Sirve además como base de referencia para cuando se testeen `LW`/`SW` "planos" (con offset inmediato) más adelante, ya que la dirección 0x200C queda con un valor conocido reutilizable.

---

# Caso 22

## Descripción
Testeo de `XOR`, operación lógica bit a bit "or exclusivo" entre dos registros. Se reutilizan los mismos operandos que en los Casos 9 y 10 (`AND`/`OR`) para poder comparar los tres resultados sobre el mismo par de valores y confirmar que la máquina no está confundiendo los func codes de las tres instrucciones lógicas.

## Instrucciones
`XOR`

## Precondiciones
- $t0 ($2) = 0x0F0F0F0F
- $t1 ($3) = 0x00FF00FF

## Code
```asm
xor $t2, $t0, $t1   # R[$t2] = R[$t0] ^ R[$t1]
```

## Postcondiciones
- Verificar $t2 ($4)
- Valor esperado: $t2 = 0x0FF00FF0

## Conclusiones
Anduvo. Comparando los tres resultados sobre el mismo par de operandos: AND=0x000F000F, OR=0x0FFF0FFF, XOR=0x0FF00FF0. Los tres son distintos entre sí y consistentes con el álgebra booleana esperada (XOR = OR sin los bits que coinciden en ambos operandos), lo que da bastante confianza de que las tres instrucciones lógicas están bien implementadas y diferenciadas.

---

# Caso 23

## Descripción
Testeo de `NOR`, negación lógica del OR bit a bit. Se reutiliza el resultado del Caso 10 (`OR` = 0x0FFF0FFF) como referencia directa: `NOR` sobre los mismos operandos debería dar exactamente el complemento a 1 de ese valor.

## Instrucciones
`NOR`

## Precondiciones
- $t0 ($2) = 0x0F0F0F0F
- $t1 ($3) = 0x00FF00FF

## Code
```asm
nor $t2, $t0, $t1   # R[$t2] = ~(R[$t0] | R[$t1])
```

## Postcondiciones
- Verificar $t2 ($4)
- Valor esperado: $t2 = 0xF0000F00

## Conclusiones
Anduvo. 0xF0000F00 es exactamente el complemento a uno de 0x0FFF0FFF (resultado del OR en el Caso 10), tal como predice la definición de NOR. Es un buen caso porque no depende de calcular nada nuevo: si el Caso 10 ya está validado, este caso se verifica por simple inspección (complementar cada dígito hex).

---

# Caso 24

## Descripción
Testeo de `SLT` (Set on Less Than, con signo), comparando un valor negativo contra uno positivo para forzar un caso donde la interpretación con signo importa.

## Instrucciones
`SLT`

## Precondiciones
- $t0 ($2) = 0xFFFFFFFF (interpretado con signo: −1)
- $t1 ($3) = 0x00000001 (1)

## Code
```asm
slt $t2, $t0, $t1   # R[$t2] = (R[$t0] < R[$t1]) con signo ? 1 : 0
```

## Postcondiciones
- Verificar $t2 ($4)
- Valor esperado: $t2 = 0x00000001

## Conclusiones
Anduvo. −1 < 1 en aritmética con signo, por lo que $t2 dio 1 tal como se esperaba. Este mismo par de operandos se reutiliza en el Caso 14 con `SLTU` para forzar el contraste con y sin signo sobre los mismos bits.

---

# Caso 25

## Descripción
Testeo de `SLTU` (Set on Less Than, sin signo), usando exactamente los mismos operandos que el Caso 13 para verificar que la interpretación sin signo da un resultado distinto — 0xFFFFFFFF sin signo es el número más grande representable en 32 bits, así que no puede ser menor que 1.

## Instrucciones
`SLTU`

## Precondiciones
- $t0 ($2) = 0xFFFFFFFF (interpretado sin signo: 4294967295)
- $t1 ($3) = 0x00000001 (1)

## Code
```asm
sltu $t2, $t0, $t1  # R[$t2] = (R[$t0] < R[$t1]) sin signo ? 1 : 0
```

## Postcondiciones
- Verificar $t2 ($4)
- Valor esperado: $t2 = 0x00000000

## Conclusiones
Anduvo. Con los mismos bits que el Caso 13, `SLTU` dio 0 en vez de 1, confirmando que la CPU efectivamente distingue la comparación con signo de la comparación sin signo y no está simplemente ignorando el func bit que las diferencia (igual que ocurre en el `mul/mulh/mulhu` o `div/divu` de la tabla, que comparten el mismo patrón con/sin signo).

---

# Caso 26

## Descripción
Testeo de `JR` (Jump Register), salto incondicional a la dirección contenida en un registro. Para no depender de conocer de antemano el mapa de memoria del programa, la estrategia es: pausar en el debugger, anotar el valor actual de `$pc`, calcular a mano la dirección de una instrucción algunas posiciones más adelante (una especie de "marcador" fácil de reconocer, por ejemplo un `nop` o `addi $zero,$zero,0`), cargarla en un registro, y saltar ahí.

## Instrucciones
`JR`

## Precondiciones
- Con el programa detenido en el debugger, leer el valor de `$pc` (ej: supongamos que da 0x00001000).
- Calcular la dirección target sumando el offset conocido hasta el marcador elegido (ej: si el marcador está 5 instrucciones después, target = 0x00001000 + 5*4 = 0x00001014).
- Cargar esa dirección en un registro:
```asm
lui  $t0, 0x0000
ori  $t0, $t0, 0x1014   # $t0 = target (ajustar según el PC real leído)
```

## Code
```asm
jr $t0        # PC = R[$t0]
```

## Postcondiciones
- Tras ejecutar (step), leer `$pc` en el debugger.
- Valor esperado: $pc == target calculado (0x00001014 en el ejemplo)
- Confirmar además que la siguiente instrucción mostrada por el debugger es efectivamente el marcador elegido, y no la instrucción que seguía secuencialmente antes del salto.

## Conclusiones
[Completar tras probar en la máquina] — Este caso depende de calcular bien la dirección target a partir del `$pc` real que muestre el debugger al momento de la prueba, por eso no se puede dar un valor numérico fijo de antemano como en los casos anteriores. Si el `$pc` resultante coincide con el target y la instrucción siguiente es el marcador, `JR` funciona correctamente.

---

# Caso 27

## Descripción
Testeo de `JALR` (Jump And Link Register), que además de saltar guarda la dirección de retorno. Ojo: la tabla A.2 lista `JALR rs rt` en la codificación pero la fórmula de operación solo menciona `R[31] = PC + 4; PC = R[rs]` — no queda claro en el manual si el campo `rt` se usa realmente como registro de destino configurable para la dirección de retorno, o si siempre escribe en `$31` ($ra) sin importar qué se ponga en `rt`. Este caso está pensado justamente para despejar esa duda.

## Instrucciones
`JALR`

## Precondiciones
- Igual que en el Caso 15: anotar `$pc` actual en el debugger (ej: 0x00002000) y cargar en un registro la dirección target de un marcador conocido (ej: 0x00002020):
```asm
lui  $t0, 0x0000
ori  $t0, $t0, 0x2020   # $t0 = target (ajustar según el PC real leído)
```

## Code
```asm
jalr $t0        # (según formato de la tabla) R[31] = PC + 4 ; PC = R[$t0]
```

## Postcondiciones
- Verificar `$pc` == target (0x00002020 en el ejemplo), igual que en el Caso 15.
- Verificar `$ra` ($31): valor esperado = dirección de la instrucción `jalr` + 4 (ej: si `jalr` estaba en 0x00001FF8, $ra debería quedar en 0x00001FFC).
- Si el assembler soporta pasarle un segundo operando (ej `jalr $rd, $t0`), repetir el caso probando si la dirección de retorno aparece en `$rd` en vez de (o además de) `$31`, para despejar la ambigüedad del campo `rt` mencionada arriba.

## Conclusiones
[Completar tras probar en la máquina] — Si `$ra` queda con PC+4 y `$pc` salta correctamente al target, la instrucción básica funciona. Falta confirmar el rol real del campo `rt`/segundo operando; si al variarlo cambia dónde se guarda la dirección de retorno, hay que corregir la descripción de la tabla A.2 (o reportarlo como posible bug/imprecisión de documentación, similar al bug de `LHU` que encontró Benicio).

---

# Caso 28

## Descripción
Testeo de `LHX` (Load Halfword indeXed, con signo), que calcula la dirección efectiva sumando dos registros (`R[rs]+R[rd]`) en vez de un registro + constante como en `LH`. Se precarga la memoria con una media palabra negativa para verificar la extensión de signo.

## Instrucciones
`LHX`, `SH` (usada solo como setup para escribir el valor conocido en memoria)

## Precondiciones
- $s0 = 0x00002000 (dirección base válida de datos)
- $s1 = 0x00000004 (offset de índice)
- $t0 = 0x00008001 (valor a precargar; su mitad baja 0x8001 tiene el bit de signo del halfword en 1)
```asm
sh $s0, $t0, 4    # M[$s0+4][15:0] = 0x8001   (precarga de memoria, EA = 0x2004)
```

## Code
```asm
lhx $t1, $s0, $s1   # R[$t1] = SignExtend( M[R[$s0]+R[$s1]][15:0] )   EA = 0x2004
```

## Postcondiciones
- Verificar $t1 mediante `examine /xw` sobre el registro
- Valor esperado: $t1 = 0xFFFF8001 (extensión de signo de 0x8001)

## Conclusiones
[Completar tras probar en la máquina] — Si $t1 da 0xFFFF8001, confirma tanto el direccionamiento indexado (base+índice, EA=0x2004) como la extensión de signo. Se recomienda correr este caso junto con el Caso 18 (`LHUX`) sobre la misma dirección de memoria para contrastar signed vs unsigned sin tener que volver a precargar el dato.

---

# Caso 29

## Descripción
Testeo de `LHUX` (Load Halfword Unsigned indeXed), variante sin signo de `LHX`. Se reutiliza la misma dirección de memoria precargada en el Caso 17 para que el único cambio entre ambos casos sea la instrucción en sí, aislando así la diferencia de comportamiento.

## Instrucciones
`LHUX`

## Precondiciones
- Continuación directa del Caso 17: memoria en 0x2004 ya contiene 0x8001 (asumiendo que se corre inmediatamente después, sin resetear la máquina)
- $s0 = 0x00002000, $s1 = 0x00000004 (mismos registros de dirección que el Caso 17)

## Code
```asm
lhux $t1, $s0, $s1   # R[$t1] = ZeroExtend( M[R[$s0]+R[$s1]][15:0] )   EA = 0x2004
```

## Postcondiciones
- Verificar $t1
- Valor esperado: $t1 = 0x00008001 (extensión con ceros, no con signo)

## Conclusiones
[Completar tras probar en la máquina] — Comparando contra el Caso 17 (mismo dato en memoria, mismo direccionamiento): si `LHX` dio 0xFFFF8001 y `LHUX` da 0x00008001, queda confirmado que la diferencia signed/unsigned funciona igual que en los shifts (Casos 2-3 y 5-6) y en `SLT`/`SLTU` (Casos 13-14) — un patrón que se repite en varios pares de instrucciones de esta ISA.

---

# Caso 30

## Descripción
Testeo de `LBX` (Load Byte indeXed, con signo), análogo a `LHX` pero para un byte en vez de un halfword.

## Instrucciones
`LBX`, `SB` (setup)

## Precondiciones
- $s0 = 0x00002000 (dirección base)
- $s1 = 0x00000008 (offset de índice, distinto al usado en los Casos 17/18 para no pisar esos datos)
- $t0 = 0x00000080 (byte negativo: bit 7 = 1)
```asm
sb $s0, $t0, 8    # M[$s0+8][7:0] = 0x80   (precarga, EA = 0x2008)
```

## Code
```asm
lbx $t1, $s0, $s1   # R[$t1] = SignExtend( M[R[$s0]+R[$s1]][7:0] )   EA = 0x2008
```

## Postcondiciones
- Verificar $t1
- Valor esperado: $t1 = 0xFFFFFF80 (extensión de signo de 0x80)

## Conclusiones
[Completar tras probar en la máquina] — Mismo criterio que el Caso 17 pero para bytes. Se recomienda correr junto con el Caso 20 (`LBUX`) sobre la misma dirección para el contraste directo signed vs unsigned.

---

# Caso 31

## Descripción
Testeo de `LBUX` (Load Byte Unsigned indeXed), variante sin signo de `LBX`, reutilizando el mismo byte precargado en el Caso 19.

## Instrucciones
`LBUX`

## Precondiciones
- Continuación directa del Caso 19: memoria en 0x2008 ya contiene 0x80
- $s0 = 0x00002000, $s1 = 0x00000008

## Code
```asm
lbux $t1, $s0, $s1   # R[$t1] = ZeroExtend( M[R[$s0]+R[$s1]][7:0] )   EA = 0x2008
```

## Postcondiciones
- Verificar $t1
- Valor esperado: $t1 = 0x00000080

## Conclusiones
[Completar tras probar en la máquina] — Igual que en el par Caso 17/18, comparar contra el Caso 19: si `LBX` dio 0xFFFFFF80 y `LBUX` da 0x00000080 sobre el mismo byte, confirma la extensión con/sin signo también para el caso de bytes.

---

# Caso 32

## Descripción
Testeo de `LWX` (Load Word indeXed) — corregido de "LWC" del pedido original, ya que esa instrucción no existe en la tabla A.2. Al ser una palabra completa (32 bits) no hay extensión de signo que testear; el foco acá es únicamente confirmar que el direccionamiento indexado (`R[rs]+R[rd]`) calcula bien la dirección efectiva.

## Instrucciones
`LWX`, `SW` (setup)

## Precondiciones
- $s0 = 0x00002000 (dirección base)
- $s1 = 0x0000000C (offset de índice, EA = 0x200C)
- $t0 = 0xCAFEBABE (palabra de 32 bits fácil de reconocer)
```asm
sw $s0, $t0, 12   # M[$s0+12] = 0xCAFEBABE   (precarga, EA = 0x200C)
```

## Code
```asm
lwx $t1, $s0, $s1   # R[$t1] = M[R[$s0]+R[$s1]]   EA = 0x200C
```

## Postcondiciones
- Verificar $t1
- Valor esperado: $t1 = 0xCAFEBABE

## Conclusiones
[Completar tras probar en la máquina] — Si $t1 recupera exactamente el mismo patrón 0xCAFEBABE que se escribió con `SW`, confirma que `LWX` calcula bien la EA indexada y que el par de escritura/lectura de memoria funciona de punta a punta para palabras completas. Sirve además como base de referencia para cuando se testeen `LW`/`SW` "planos" (con offset inmediato) más adelante, ya que la dirección 0x200C queda con un valor conocido reutilizable.

---

# Casos de Prueba — Instrucciones Tipo I


# Caso 33

## Descripción
Testeo de `ADDI` (Add Immediate), suma entre un registro y una constante inmediata de 17 bits en complemento a 2. Se usa una constante negativa para verificar explícitamente la extensión de signo (`SE`), ya que con una constante positiva no se podría distinguir de un `ADDIU` sin signo (que esta ISA no tiene, pero conviene confirmar que el signo se maneja bien de todas formas).

## Instrucciones
`ADDI`

## Precondiciones
- $t0 ($2) = 0x00000005 (5)

## Code
```asm
addi $t0, $t1, -8    # R[$t1] = R[$t0] + SE(-8)
```

## Postcondiciones
- Verificar $t1 ($3) mediante `examine /xw $3 1`
- Valor esperado: $t1 = 0xFFFFFFFD (−3)

## Conclusiones
[Completar tras probar en la máquina] — 5 + (−8) = −3 = 0xFFFFFFFD. Si en cambio el resultado diera 0x00000FF8 o similar (imm tratado como sin signo/zero-extendido), indicaría que `ADDI` no está aplicando `SE(imm)` correctamente.

---

# Casos de Prueba — Instrucciones Tipo J

# Caso 34

## Descripción
Testeo de `J` (Jump incondicional). La dirección de destino se reconstruye combinando los 3 bits más significativos del PC vigente al momento del salto con el campo de 27 bits `address` de la instrucción (más 2 ceros fijos al final). Al no ser un salto relativo, alcanza con indicarle al ensamblador una dirección destino dentro de la misma región de 512MB que el PC actual.

## Instrucciones
`J`

## Precondiciones
- Con el programa detenido en el debugger, anotar el `$pc` actual (ej: 0x00001000).
- Elegir una dirección "marcador" alineada a 4 bytes, dentro de la misma región de 512MB (mismos 3 bits más altos que el PC actual), por ejemplo 0x00001020 (8 instrucciones más adelante). Confirmar en el debugger que en esa dirección hay una instrucción reconocible (ej. un `nop`).

## Code
```asm
j 0x00001020   # PC = E(address); se asume que el ensamblador acepta la dirección numérica/etiqueta y calcula "address" automáticamente
```

## Postcondiciones
- Tras step, verificar `$pc`: valor esperado = 0x00001020.
- Confirmar que la siguiente instrucción mostrada es efectivamente el marcador elegido.

## Conclusiones
[Completar tras probar en la máquina] — Si `$pc` termina exactamente en la dirección marcada, confirma que la reconstrucción de dirección (3 bits del PC + 27 bits de `address` + 2 ceros) funciona. Si el ensamblador no acepta direcciones numéricas directas, hay que rearmar el caso con una etiqueta (`marcador:`) en vez del valor 0x00001020 literal.

---

# Caso 35

## Descripción
Testeo de `JAL` (Jump And Link), variante de `J` que además guarda la dirección de retorno en `$ra` ($31). Mismo mecanismo de reconstrucción de dirección que el Caso 2.

## Instrucciones
`JAL`

## Precondiciones
- Con el programa detenido en el debugger, anotar `$pc` actual (ej: 0x00002000).
- Elegir dirección destino marcador dentro de la misma región de 512MB, ej: 0x00002040.

## Code
```asm
jal 0x00002040   # R[31] = PC + 4 ; PC = E(address)
```

## Postcondiciones
- Verificar `$pc`: valor esperado = 0x00002040.
- Verificar `$ra` ($31): valor esperado = dirección de la instrucción `jal` + 4 (ej: si `jal` estaba en 0x00002000, `$ra` debería quedar en 0x00002004).

## Conclusiones
[Completar tras probar en la máquina] — Si ambos valores coinciden, confirma que `JAL` no solo salta correctamente sino que además guarda bien la dirección de retorno, algo que `J` no hace (comparar directamente contra el Caso 2 usando la misma técnica es una buena forma de aislar el "+ link" como la única diferencia real entre ambas instrucciones).

---
# Casos de Prueba — Instrucciones Tipo L

# Caso 36

## Descripción
Testeo de `ANDI`/`ANDI.H` (AND lógico con constante inmediata de 16 bits, aplicada a la mitad baja o alta de la palabra según el bit `h`). Se usa un valor con nibbles distintos en ambas mitades (`0x12345678`) para poder verificar por separado el comportamiento de `h=0` y `h=1` sobre el mismo registro fuente.

## Instrucciones
`ANDI`, `ANDI.H` (o sintaxis equivalente que use el ensamblador para el bit `h`)

## Precondiciones
- $t0 ($2) = 0x12345678

## Code
```asm
andi   $t0, $t1, 0x0F0F   # h=0: R[$t1] = R[$t0] & ZE(0x0F0F)  -> afecta solo la mitad baja
andi.h $t0, $t2, 0x0F0F   # h=1: R[$t2] = R[$t0] & ZC(0x0F0F)  -> afecta solo la mitad alta
```

## Postcondiciones
- Verificar $t1 ($3): valor esperado = 0x00000608
- Verificar $t2 ($4): valor esperado = 0x02040000

## Conclusiones
[Completar tras probar en la máquina] — Con `h=0`, el resultado solo debería reflejar el AND entre la mitad baja de `$t0` (0x5678) y la constante, quedando la mitad alta en 0. Con `h=1`, es al revés: se opera sobre la mitad alta (0x1234) y la mitad baja queda en 0. **Ojo**: el manual advierte explícitamente que "actualmente hay un bug importante a solucionar en la instrucción ANDI" — si los valores no coinciden con lo esperado acá, puede no ser un error de este caso sino el bug ya reportado por la cátedra; conviene avisar en el foro con el valor exacto observado.

---

# Caso 37

## Descripción
Testeo de `ORI`/`ORI.H`, análogo al Caso 4 pero con OR lógico. Se reutiliza el mismo valor fuente (`0x12345678`) para poder comparar directamente contra `ANDI`/`ANDI.H` y `XORI`/`XORI.H` (Caso 6) sobre los mismos bits.

## Instrucciones
`ORI`, `ORI.H`

## Precondiciones
- $t0 ($2) = 0x12345678

## Code
```asm
ori   $t0, $t1, 0x0F0F   # h=0: R[$t1] = R[$t0] | ZE(0x0F0F)
ori.h $t0, $t2, 0x0F0F   # h=1: R[$t2] = R[$t0] | ZC(0x0F0F)
```

## Postcondiciones
- Verificar $t1 ($3): valor esperado = 0x12345F7F
- Verificar $t2 ($4): valor esperado = 0x1F3F5678

## Conclusiones
[Completar tras probar en la máquina] — Con `h=0`, solo la mitad baja debería verse afectada (0x5678 → 0x5F7F), quedando la mitad alta intacta (0x1234). Con `h=1`, es al revés (mitad alta 0x1234→0x1F3F, mitad baja intacta 0x5678). Comparar contra el Caso 4 sobre los mismos bits de entrada: donde `ANDI` "achica" el valor (deja solo la intersección), `ORI` lo "agranda" (deja la unión) — mismo patrón que ya se vio con `AND`/`OR` tipo R.

---

# Caso 38

## Descripción
Testeo de `XORI`/`XORI.H` ("XORI/H" en la tabla A.1; puede aparecer también escrito como "HORI/H" por error de tipeo, pero la tabla A.1 solo lista `XORI/H`), análogo a los Casos 4 y 5 pero con OR exclusivo. Mismo valor fuente que los casos anteriores para completar la comparación de las tres instrucciones lógicas inmediatas sobre los mismos datos.

## Instrucciones
`XORI`, `XORI.H`

## Precondiciones
- $t0 ($2) = 0x12345678

## Code
```asm
xori   $t0, $t1, 0x0F0F   # h=0: R[$t1] = R[$t0] ^ ZE(0x0F0F)
xori.h $t0, $t2, 0x0F0F   # h=1: R[$t2] = R[$t0] ^ ZC(0x0F0F)
```

## Postcondiciones
- Verificar $t1 ($3): valor esperado = 0x12345977
- Verificar $t2 ($4): valor esperado = 0x1D3B5678

## Conclusiones
[Completar tras probar en la máquina] — Comparando los tres resultados sobre el mismo `0x12345678` y la misma constante `0x0F0F`: `ANDI`=0x00000608 (Caso 4), `ORI`=0x12345F7F (Caso 5), `XORI`=0x12345977 (este caso) para `h=0`; y análogamente para `h=1`. Si los tres difieren de forma consistente con el álgebra booleana esperada, da confianza de que las tres variantes lógicas inmediatas están bien diferenciadas — igual que se hizo con `AND`/`OR`/`XOR` tipo R.

---

# Caso 39

## Descripción
Testeo de `LUI` (Load Upper Immediate), que carga una constante de 16 bits en la mitad alta del registro destino, dejando la mitad baja en 0. Es la instrucción clave para poder cargar direcciones/constantes arbitrarias de 32 bits combinada con `ORI` (como ya se usó de forma implícita en los Casos de `JR`/`JALR` de la tanda anterior).

## Instrucciones
`LUI`

## Precondiciones
- Ninguna especial — no depende de valores previos en registros.

## Code
```asm
lui $t0, 0xABCD   # R[$t0] = ZC(0xABCD) = {0xABCD, 16'b0}
```

## Postcondiciones
- Verificar $t0 ($2): valor esperado = 0xABCD0000

## Conclusiones
[Completar tras probar en la máquina] — Si el registro queda exactamente en 0xABCD0000 (mitad baja en ceros), confirma el comportamiento de `LUI`. Se puede extender este caso agregando una segunda instrucción `ori $t0, $t1, 0x1234` para verificar que la combinación `LUI+ORI` arma correctamente la constante completa 0xABCD1234, técnica que ya se usó implícitamente en los casos de `JR`/`JALR`/`TRAP` anteriores.

---

# Casos de Prueba — Instrucciones Tipo I

# Caso 40

## Descripción
Testeo de `LW` (Load Word) con direccionamiento indexado por offset inmediato (no indexado por registro como `LWX`, ya testeado antes). Se usa `SW` únicamente como setup para dejar un valor conocido en memoria.

## Instrucciones
`LW`, `SW` (setup)

## Precondiciones
- $s0 = 0x00002000 (dirección base)
- $t0 = 0xCAFEBABE (valor a precargar)
```asm
sw $s0, $t0, 16   # M[EA($s0,16)] = 0xCAFEBABE   (EA = 0x2010)
```

## Code
```asm
lw $s0, $t1, 16   # R[$t1] = M[EA($s0, 16)]   (EA = 0x2010)
```

## Postcondiciones
- Verificar $t1: valor esperado = 0xCAFEBABE

## Conclusiones
[Completar tras probar en la máquina] — Si $t1 recupera exactamente 0xCAFEBABE, confirma que `LW` con offset inmediato calcula bien la EA (base + desplazamiento con signo) y que el par de escritura/lectura de memoria funciona igual que ya se validó con `LWX` (direccionamiento indexado por registro) en la tanda anterior.

---

# Caso 41

## Descripción
Testeo de `SW` (Store Word) de forma aislada, verificando directamente en memoria con `examine` en vez de leer de vuelta con `LW`, para no depender de que `LW` también esté funcionando bien.

## Instrucciones
`SW`

## Precondiciones
- $s0 = 0x00002000 (dirección base)
- $t0 = 0x11223344 (valor a almacenar)

## Code
```asm
sw $s0, $t0, 20   # M[EA($s0, 20)] = 0x11223344   (EA = 0x2014)
```

## Postcondiciones
- Verificar memoria directamente: `examine /xw 0x2014 1`
- Valor esperado en 0x2014: 0x11223344

## Conclusiones
[Completar tras probar en la máquina] — Verificar la memoria directamente (en vez de con `LW`) aísla el test: si el valor en 0x2014 es correcto, `SW` funciona independientemente de si `LW` tiene o no algún problema propio.

---

# Caso 42

## Descripcion
Testeo de `SH` (Store Halfword), que solo escribe los 16 bits menos significativos del registro fuente en memoria, sin tocar la otra mitad de la palabra en memoria.

## Instrucciones
`SH`

## Precondiciones
- $s0 = 0x00002000
- $t0 = 0xFFFFBEEF (solo interesa la mitad baja, 0xBEEF)
- Memoria en 0x2018 inicializada previamente en 0x00000000 (ej. mediante `sw $s0, $zero, 24`), para poder confirmar que `SH` no toca la mitad alta de la palabra en memoria.

## Code
```asm
sh $s0, $t0, 24   # M[EA($s0, 24)][15:0] = R[$t0][15:0] = 0xBEEF   (EA = 0x2018)
```

## Postcondiciones
- Verificar memoria: `examine /xh 0x2018 1` → valor esperado = 0xBEEF
- Verificar la palabra completa: `examine /xw 0x2018 1` → valor esperado = 0x0000BEEF (si `SH` respeta la otra mitad y la memoria arrancó en 0)

## Conclusiones
[Completar tras probar en la máquina] — Si la media palabra escrita coincide con 0xBEEF y la palabra completa queda en 0x0000BEEF (sin "basura" en la mitad alta), confirma que `SH` escribe únicamente los 16 bits indicados y no pisa el resto de la palabra en memoria.

---

# Caso 43

## Descripción
Testeo de `SB` (Store Byte), análogo al Caso 10 pero para un único byte.

## Instrucciones
`SB`

## Precondiciones
- $s0 = 0x00002000
- $t0 = 0xFFFFFF7A (solo interesa el byte bajo, 0x7A)
- Memoria en 0x201C inicializada en 0x00000000 previamente (ej. `sw $s0, $zero, 28`)

## Code
```asm
sb $s0, $t0, 28   # M[EA($s0, 28)][7:0] = R[$t0][7:0] = 0x7A   (EA = 0x201C)
```

## Postcondiciones
- Verificar memoria: `examine /xb 0x201C 1` → valor esperado = 0x7A
- Verificar la palabra completa: `examine /xw 0x201C 1` → valor esperado = 0x0000007A

## Conclusiones
[Completar tras probar en la máquina] — Igual criterio que el Caso 10: si solo el byte bajo cambia y el resto de la palabra sigue en 0, confirma que `SB` escribe exactamente 8 bits sin afectar el resto.

---

# Caso 44

## Descripción
Testeo de `LH` (Load Halfword, con signo). Se precarga con `SH` una media palabra cuyo bit más significativo (bit 15) está en 1, para verificar la extensión de signo al cargarla en un registro de 32 bits.

## Instrucciones
`LH`, `SH` (setup)

## Precondiciones
- $s0 = 0x00002000
- $t0 = 0x00008421 (mitad baja 0x8421, bit de signo del halfword en 1)
```asm
sh $s0, $t0, 32   # M[EA($s0,32)][15:0] = 0x8421   (EA = 0x2020)
```

## Code
```asm
lh $s0, $t1, 32   # R[$t1] = SHE(M[EA($s0,32)][15:0])   (EA = 0x2020)
```

## Postcondiciones
- Verificar $t1: valor esperado = 0xFFFF8421

## Conclusiones
[Completar tras probar en la máquina] — Si $t1 da 0xFFFF8421 (extensión de signo de 0x8421), confirma la extensión con signo de `LH`. Se recomienda correr el Caso 13 (`LHU`) sobre la misma dirección de memoria para contrastar signed vs unsigned sin volver a precargar el dato.

---

# Caso 45

## Descripción
Testeo de `LHU` (Load Halfword Unsigned), reutilizando la misma dirección precargada en el Caso 12 para aislar la única diferencia entre ambas instrucciones (la extensión con o sin signo). Es particularmente relevante testear esta instrucción con cuidado ya que la cátedra confirmó un bug histórico en `LHU` relacionado con el "ending" de la instrucción.

## Instrucciones
`LHU`

## Precondiciones
- Continuación directa del Caso 12: memoria en 0x2020 ya contiene 0x8421.
- $s0 = 0x00002000 (mismo registro base)

## Code
```asm
lhu $s0, $t1, 32   # R[$t1] = ZE(M[EA($s0,32)][15:0])   (EA = 0x2020)
```

## Postcondiciones
- Verificar $t1: valor esperado = 0x00008421

## Conclusiones
[Completar tras probar en la máquina] — Comparando contra el Caso 12 (mismo dato en memoria): si `LH` dio 0xFFFF8421 y `LHU` da 0x00008421, confirma que la diferencia signed/unsigned funciona. Este caso es especialmente sensible porque el bug reportado y corregido por la cátedra fue justamente que el "ending" de `LHU` no estaba y el flujo seguía ejecutando instrucciones de una sección de código que no correspondía — si el `$pc` termina en un lugar inesperado después de este `LHU` (no en la instrucción siguiente esperada), puede tratarse de una regresión del mismo bug.

---

# Caso 46

## Descripción
Testeo de `LB` (Load Byte, con signo), análogo al Caso 12 pero para un byte.

## Instrucciones
`LB`, `SB` (setup)

## Precondiciones
- $s0 = 0x00002000
- $t0 = 0x00000091 (byte bajo 0x91, bit de signo en 1)
```asm
sb $s0, $t0, 36   # M[EA($s0,36)][7:0] = 0x91   (EA = 0x2024)
```

## Code
```asm
lb $s0, $t1, 36   # R[$t1] = SBE(M[EA($s0,36)][7:0])   (EA = 0x2024)
```

## Postcondiciones
- Verificar $t1: valor esperado = 0xFFFFFF91

## Conclusiones
[Completar tras probar en la máquina] — Mismo criterio que el Caso 12 pero para bytes. Se recomienda correr junto con el Caso 15 (`LBU`) sobre la misma dirección para el contraste directo.

---

# Caso 47

## Descripción
Testeo de `LBU` (Load Byte Unsigned), reutilizando el byte precargado en el Caso 14.

## Instrucciones
`LBU`

## Precondiciones
- Continuación directa del Caso 14: memoria en 0x2024 ya contiene 0x91.
- $s0 = 0x00002000

## Code
```asm
lbu $s0, $t1, 36   # R[$t1] = ZBE(M[EA($s0,36)][7:0])   (EA = 0x2024)
```

## Postcondiciones
- Verificar $t1: valor esperado = 0x00000091

## Conclusiones
[Completar tras probar en la máquina] — Igual que en el par Caso 12/13, comparar contra el Caso 14: si `LB` dio 0xFFFFFF91 y `LBU` da 0x00000091 sobre el mismo byte, confirma la extensión con/sin signo también para bytes.

---

# Caso 48

## Descripción
Testeo de `BEQ` (Branch if Equal). Al ser un salto relativo al PC, se usa la misma técnica de los Casos de `JR`/`JALR`: anotar el `$pc` antes de ejecutar el branch y calcular a mano el target esperado según la fórmula `PC = PC + 4 + Branch(imm)`, con `Branch(imm) = imm * 4` (para `imm` positivo y pequeño).

## Instrucciones
`BEQ`

## Precondiciones
- Con el programa detenido en el debugger, anotar `$pc` actual (ej: 0x00001000).
- $t0 ($2) = 0x00000007
- $t1 ($3) = 0x00000007 (igual a $t0, para forzar la condición verdadera)

## Code
```asm
beq $t0, $t1, 2   # if (R[$t0]==R[$t1]) PC = PC+4+Branch(2) = PC+4+8
```

## Postcondiciones
- Tras step, verificar `$pc`: valor esperado = PC_inicial + 12 (ej: 0x0000100C si el branch estaba en 0x00001000)

## Conclusiones
[Completar tras probar en la máquina] — Con $t0==$t1, la condición es verdadera y el salto debería ejecutarse, saltando a PC+12. Si el `$pc` en cambio avanza solo +4 (como si el branch no se tomara), la condición de igualdad no se está evaluando bien.

---

# Caso 49

## Descripción
Testeo de `BNE` (Branch if Not Equal), usando la misma técnica del Caso 16 pero con operandos distintos para forzar la condición verdadera.

## Instrucciones
`BNE`

## Precondiciones
- Anotar `$pc` actual (ej: 0x00001100).
- $t0 ($2) = 0x00000005
- $t1 ($3) = 0x00000009 (distinto de $t0)

## Code
```asm
bne $t0, $t1, 2   # if (R[$t0]!=R[$t1]) PC = PC+4+Branch(2) = PC+4+8
```

## Postcondiciones
- Verificar `$pc`: valor esperado = PC_inicial + 12

## Conclusiones
[Completar tras probar en la máquina] — Con $t0 != $t1 el salto debería tomarse. Complementario al Caso 16: si se corriera `BNE` con los mismos operandos del Caso 16 ($t0==$t1), el salto NO debería tomarse (`$pc` avanzaría solo +4) — buen caso adicional opcional para confirmar ambos caminos de la condición.

---

# Caso 50

## Descripción
Testeo de `BLT` (Branch if Less Than, con signo).

## Instrucciones
`BLT`

## Precondiciones
- Anotar `$pc` actual (ej: 0x00001200).
- $t0 ($2) = 0x00000003 (3)
- $t1 ($3) = 0x00000008 (8)

## Code
```asm
blt $t0, $t1, 2   # if (R[$t0] < R[$t1]) PC = PC+4+Branch(2) = PC+4+8
```

## Postcondiciones
- Verificar `$pc`: valor esperado = PC_inicial + 12

## Conclusiones
[Completar tras probar en la máquina] — 3 < 8, condición verdadera, el salto debería tomarse. Repetir con un caso de signos opuestos (ej. $t0 negativo, $t1 positivo) sería un buen complemento opcional para confirmar que la comparación es realmente con signo y no solo bit a bit sin signo.

---

# Caso 51

## Descripción
Testeo de `BGT` (Branch if Greater Than, con signo), reutilizando los mismos operandos del Caso 18 pero invertidos para forzar la condición contraria.

## Instrucciones
`BGT`

## Precondiciones
- Anotar `$pc` actual (ej: 0x00001300).
- $t0 ($2) = 0x00000008 (8)
- $t1 ($3) = 0x00000003 (3)

## Code
```asm
bgt $t0, $t1, 2   # if (R[$t0] > R[$t1]) PC = PC+4+Branch(2) = PC+4+8
```

## Postcondiciones
- Verificar `$pc`: valor esperado = PC_inicial + 12

## Conclusiones
[Completar tras probar en la máquina] — 8 > 3, condición verdadera. Comparado contra el Caso 18 (mismos valores pero invertidos y con `BLT`), ambos deberían tomar el salto, lo cual es esperable y consistente: 3<8 y 8>3 son la misma relación vista desde ambos lados.

---

# Caso 52

## Descripción
Testeo de `BLE` (Branch if Less or Equal, con signo), usando operandos iguales para forzar específicamente el caso límite de la igualdad (el caso estrictamente menor ya quedó cubierto conceptualmente por `BLT` en el Caso 18).

## Instrucciones
`BLE`

## Precondiciones
- Anotar `$pc` actual (ej: 0x00001400).
- $t0 ($2) = 0x00000005 (5)
- $t1 ($3) = 0x00000005 (5, igual a $t0)

## Code
```asm
ble $t0, $t1, 2   # if (R[$t0] <= R[$t1]) PC = PC+4+Branch(2) = PC+4+8
```

## Postcondiciones
- Verificar `$pc`: valor esperado = PC_inicial + 12

## Conclusiones
[Completar tras probar en la máquina] — Con $t0 == $t1, si el salto se toma (`$pc` avanza +12), confirma que `BLE` es inclusivo (`<=`) y no un `BLT` mal etiquetado. Este es el caso más importante para diferenciar `BLE` de `BLT`, ya que con operandos estrictamente distintos ambas instrucciones se comportarían igual.

---

# Caso 53

## Descripción
Testeo de `BGE` (Branch if Greater or Equal, con signo), mismo criterio que el Caso 20 pero para el otro extremo inclusivo.

## Instrucciones
`BGE`

## Precondiciones
- Anotar `$pc` actual (ej: 0x00001500).
- $t0 ($2) = 0x00000005 (5)
- $t1 ($3) = 0x00000005 (5, igual a $t0)

## Code
```asm
bge $t0, $t1, 2   # if (R[$t0] >= R[$t1]) PC = PC+4+Branch(2) = PC+4+8
```

## Postcondiciones
- Verificar `$pc`: valor esperado = PC_inicial + 12

## Conclusiones
[Completar tras probar en la máquina] — Igual razonamiento que el Caso 20: con operandos iguales, si el salto se toma confirma que `BGE` es inclusivo. Corriendo los Casos 20 y 21 con los mismos operandos, ambos deberían tomar el salto (a diferencia de `BLT`/`BGT` con operandos iguales, que no deberían tomarlo) — un buen caso adicional opcional sería repetir `BLT`/`BGT` con $t0==$t1 para confirmar que esos, a diferencia de `BLE`/`BGE`, NO saltan.

---

# Caso 54

## Descripción
Testeo de `SLTI` (Set Less Than Immediate, con signo), comparando un registro negativo contra una constante inmediata positiva pequeña, para forzar un caso donde la interpretación con signo importa. Se reutiliza el mismo par lógico que ya se usó para `SLT`/`SLTU` en la tanda anterior, ahora con constante inmediata en vez de segundo registro.

## Instrucciones
`SLTI`

## Precondiciones
- $t0 ($2) = 0xFFFFFFFF (interpretado con signo: −1)

## Code
```asm
slti $t0, $t1, 1   # R[$t1] = (R[$t0] < SE(1)) ? 1 : 0   (con signo)
```

## Postcondiciones
- Verificar $t1 ($3): valor esperado = 0x00000001

## Conclusiones
[Completar tras probar en la máquina] — Con signo, −1 < 1, por lo que $t1 debería dar 1. Este mismo par de valores se reutiliza en el Caso 23 (`SLTIU`) para forzar el contraste con y sin signo, igual que se hizo con `SLT`/`SLTU`.

---

# Caso 55

## Descripción
Testeo de `SLTIU` (Set Less Than Immediate Unsigned), usando exactamente el mismo registro fuente y constante que el Caso 22 para verificar que la interpretación sin signo da un resultado distinto — 0xFFFFFFFF sin signo es el número más grande representable en 32 bits, por lo que no puede ser menor que 1.

## Instrucciones
`SLTIU`

## Precondiciones
- $t0 ($2) = 0xFFFFFFFF (interpretado sin signo: 4294967295)

## Code
```asm
sltiu $t0, $t1, 1   # R[$t1] = (R[$t0] < ZE(1)) ? 1 : 0   (sin signo)
```

## Postcondiciones
- Verificar $t1 ($3): valor esperado = 0x00000000

## Conclusiones
[Completar tras probar en la máquina] — Con los mismos bits que el Caso 22, `SLTIU` debería dar 0 en vez de 1, confirmando que la CPU distingue la comparación con signo de la sin signo también en la versión inmediata, igual que ya se confirmó con `SLT`/`SLTU` (tipo R) y `MUL`/`MULH`/`MULHU` y `DIV`/`DIVU` (tanda anterior).

---
