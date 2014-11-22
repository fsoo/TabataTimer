/*
 * CpuCritical.h
 *
 *  Created on: May 2, 2014
 *      Author: fsoo
 */

#ifndef CPUCRITICAL_H_
#define CPUCRITICAL_H_

/*
 * updated EnterCritical and ExitCritical
 * fixes bug noted in
/* http://mcuoneclipse.com/2014/01/26/entercritical-and-exitcritical-why-things-are-failing-badly/
 * usage: put CpuCriticalVar() at top of function
 * then use CpuEnterCritical before and after critical section
 * avoids possible interrupt due to use of global counter CpuSR in original code
 */
#define CpuCriticalVar()  uint8_t cpuSR
#define CpuEnterCritical()              \
  do {                                  \
    asm (                               \
    "MRS   R0, PRIMASK\n\t"             \
    "CPSID I\n\t"                       \
    "STRB R0, %[output]"                \
    : [output] "=m" (cpuSR) :: "r0");   \
  } while(0)

#define CpuExitCritical()               \
  do{                                   \
    asm (                               \
    "ldrb r0, %[input]\n\t"             \
    "msr PRIMASK,r0;\n\t"               \
    ::[input] "m" (cpuSR) : "r0");      \
  } while(0)
#endif /* CPUCRITICAL_H_ */
