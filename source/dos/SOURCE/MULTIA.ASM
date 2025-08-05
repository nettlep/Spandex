; Originally released under a custom license.
; This historical re-release is provided under the MIT License.
; See the LICENSE file in the repo root for details.
;
; https://github.com/nettlep

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
;±                                                                          ±
;±   Copyright (c) 1996 Paul D. Nettle.  All Rights Reserved.               ±
;±                                                                          ±
;±   [Multia.asm  ] - Multi-tasker for DOS                                  ±
;±                                                                          ±
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

        .486
        .Model Flat, PASCAL
        .Data

        PUBLIC  _MLTIInit_
        PUBLIC  _MLTIUninit_
        PUBLIC  MLTIFork_
        PUBLIC  MLTISetPriority_
        PUBLIC  MLTISetProcessActive_
        PUBLIC  MLTISemaphoreSet_
        PUBLIC  MLTISemaphoreGet_
        PUBLIC  MLTIInterrupt_

        PUBLIC  _MLTICurrentProcess
        PUBLIC  _MLTIProcessCount
        PUBLIC  _MLTITickCounter
        PUBLIC  _MLTISpeedSetting
        PUBLIC  _MLTIChainInterrupt

        EXTRN   __STACKLOW                      ;These are Watcom global vars
        EXTRN   __STACKTOP                      ;

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

S_EAX           equ     0                       ;Offsets into Task State Table
S_EBX           equ     4                       ;
S_ECX           equ     8                       ;
S_EDX           equ     12                      ;
S_ESI           equ     16                      ;
S_EDI           equ     20                      ;
S_ESP           equ     24                      ;
S_EBP           equ     28                      ;
S_EIP           equ     32                      ;
S_DS            equ     36                      ;
S_ES            equ     40                      ;
S_SS            equ     44                      ;
S_CS            equ     48                      ;
S_GS            equ     52                      ;
S_FS            equ     56                      ;
S_FLAGS         equ     60                      ;

TST_SAVE_SIZE   equ     64                      ;Length of modifiable section

S_SSIZE         equ     64                      ;Non-modifiable
S_PRIORITY      equ     68                      ;
S_ACTIVE        equ     72                      ;

TST_SIZE        equ     76                      ;Length incl non-modify section

MAX_PROCS       equ     64
MAX_SEMAPHORES  equ     1000

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

align 2

_MLTIProcessCount       dd      1
_MLTICurrentProcess     dd      0
_MLTITickCounter        dd      0
_MLTIChainInterrupt     dq      0
_MLTISpeedSetting       dd      0

ChainCounter            dd      0
CurrentProcessPriority  dd      0
StackUsed               dd      0
StackStart              dd      0
ReservedStack           dd      0
TotalStack              dd      0

TST                     dd      MAX_PROCS * 128 dup(0)
SEM                     dd      MAX_SEMAPHORES * 2 dup(0)

t_eax                   dd      ?
t_ebx                   dd      ?
t_ecx                   dd      ?
t_edx                   dd      ?
t_esi                   dd      ?
t_edi                   dd      ?
t_esp                   dd      ?
t_ebp                   dd      ?
t_eip                   dd      ?
t_ds                    dd      ?
t_es                    dd      ?
t_ss                    dd      ?
t_cs                    dd      ?
t_gs                    dd      ?
t_fs                    dd      ?
t_flags                 dd      ?
t_ssize                 dd      ?
t_priority              dd      ?
t_active                dd      ?

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

        .Code

align 4
_MLTIInit_              Proc PASCAL Near

;eax -> reserved stack space for parent process

        mov     ReservedStack,eax               ;Reserve space for parent
        mov     StackUsed,eax                   ;

        mov     eax,__STACKTOP                  ;Track the total stack space
        sub     eax,__STACKLOW                  ;
        mov     TotalStack,eax                  ;
        mov     eax,__STACKLOW                  ;
        mov     StackStart,eax                  ;
        ret                                     ;

_MLTIInit_              EndP

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

_MLTIUninit_            Proc PASCAL Near

        ;Currently, this routine is a stub...maybe some day this will
        ;     need an uninit routine... but now, it don't. :)

        ret

_MLTIUninit_            EndP

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

align 4
MLTIFork_               Proc PASCAL Near

;eax -> stack size
;edx -> Priority

        pushfd                                  ;Save these...
        cli                                     ; (shut these off)
        pop     t_flags                         ;
        push    _MLTIProcessCount               ; (return code for the new proc)
        pop     t_eax                           ;
        mov     t_ebx,ebx                       ;
        mov     t_ecx,ecx                       ;
        mov     t_edx,edx                       ;
        mov     t_esi,esi                       ;
        mov     t_edi,edi                       ;
        mov     t_ebp,ebp                       ;
        pop     t_eip                           ;
        push    t_eip                           ;
        mov     t_ds,ds                         ;
        mov     t_es,es                         ;
        mov     t_ss,ss                         ;
        mov     t_cs,cs                         ;
        mov     t_gs,gs                         ;
        mov     t_fs,fs                         ;
        mov     t_ssize,eax                     ;
        mov     t_priority,edx                  ;
        mov     t_active,1                      ;

        add     eax,StackUsed                   ;Verify no stack overrun
        cmp     eax,TotalStack                  ;
        jl      MF_StackOK                      ;
        mov     eax,-1                          ;
        jmp     MF_Exit                         ;

MF_StackOK:
        mov     eax,_MLTIProcessCount           ;Verify no proc count overrun
        cmp     eax,MAX_PROCS                   ;
        jl      MF_ProcsOK                      ;
        mov     eax,-1                          ;
        jmp     MF_Exit                         ;

MF_ProcsOK:
        push    ebx                             ;Save these
        push    ecx                             ;
        push    edx                             ;
        push    esi                             ;
        push    edi                             ;

        mov     eax,StackStart                  ;Locate new stack
        add     eax,StackUsed                   ;
        mov     t_esp,eax                       ;
        mov     eax,t_ssize                     ;
        add     StackUsed,eax                   ;

        mov     ecx,t_ssize                     ;Copy the stack's contents
        mov     edi,t_esp                       ;
        sub     edi,ecx                         ;
        mov     esi,esp                         ;
        sub     esi,ecx                         ;
        shr     ecx,2                           ;
        cld                                     ;
        rep     movsd                           ;

        mov     edi,_MLTIProcessCount           ;Save temps into the new TST
        shl     edi,7                           ;
        add     edi,offset TST                  ;
        mov     esi,offset t_eax                ;
        mov     ecx,TST_SIZE/4                  ;
        rep     movsd                           ;

        inc     _MLTIProcessCount               ;Indicate the new process

        mov     eax,_MLTICurrentProcess         ;Return the current process

        pop     edi                             ;Restore these
        pop     esi                             ;
        pop     edx                             ;
        pop     ecx                             ;
        pop     ebx                             ;

MF_Exit:
        sti                                     ;Laters...
        ret                                     ;

MLTIFork_               EndP

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

align 4
MLTISetPriority_        Proc PASCAL Near

;eax -> Process ID
;edx -> Priority
uses    edi

        cli                                     ;Shut these off

        mov     edi,eax                         ;Save the priority
        shl     edi,7                           ;
        add     edi,offset TST                  ;
        mov     [edi+S_PRIORITY],edx            ;

        sti                                     ;Laters...
        ret                                     ;

MLTISetPriority_        EndP

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

align 4
MLTISetProcessActive_   Proc PASCAL Near

;eax -> Process ID
;edx -> Process activity flag

uses    edi

        cli                                     ;Shut these off

        mov     edi,eax                         ;Set process active flag
        shl     edi,7                           ;
        add     edi,offset TST                  ;
        mov     [edi+S_ACTIVE],edx              ;

        sti                                     ;Laters...
        ret                                     ;

MLTISetProcessActive_   EndP

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

align 4
MLTISemaphoreSet_       Proc PASCAL Near

;eax -> New value
;ebx -> Pointer to int that receives previous owner's ID

uses    ecx,edi

        cli                                     ;Shut these off

        mov     edi,eax                         ;XCHG new val with old
        shl     edi,3                           ;
        add     edi,offset SEM                  ;
        xchg    [edi],eax                       ;

        mov     ecx,[edi+4]                     ;Save off previous owner
        mov     [ebx],ecx                       ;

        mov     ebx,_MLTICurrentProcess         ;Save off new owner
        mov     [edi+4],ebx                     ;

        sti                                     ;Laters...
        ret                                     ;

MLTISemaphoreSet_       EndP

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

align 4
MLTISemaphoreGet_       Proc PASCAL Near

;eax -> Pointer to int that receives previous owner's ID

uses    ebx,edi

        cli                                     ;Shut these off

        mov     edi,eax                         ;Get the value and owner
        shl     edi,3                           ;
        add     edi,offset SEM                  ;
        mov     ebx,[edi+4]                     ;
        mov     [eax],ebx                       ;
        mov     eax,[edi]                       ;

        sti                                     ;Laters...
        ret                                     ;

MLTISemaphoreGet_       EndP

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

align 4
MLTIInterrupt_          Proc PASCAL Near

        cli                                     ;Recognize the interrupt
        push    ds                              ;Set this stuff up
        push    DGROUP                          ;
        push    DGROUP                          ;
        pop     ds                              ;

        inc     _MLTITickCounter                ;For the user...

        push    eax                             ;Time to chain interrupts?
        mov     al,20h                          ;
        out     20h,al                          ;
        mov     eax,_MLTISpeedSetting           ;
        sub     ChainCounter,eax                ;
        pop     eax                             ;
        jnc     MI_NoChain                      ;

        add     ChainCounter,0ffffh             ;Reset counter

        pushfd                                  ;Do the chain thing...
        call    [fword ptr _MLTIChainInterrupt] ;

MI_NoChain:
        sub     CurrentProcessPriority,1        ;Check for Priority countdown
        jc      MI_Switch                       ;

        pop     ds                              ;QuickExit... no switch
        sti                                     ;
        iretd                                   ;

MI_Switch:
        mov     t_eax,eax                       ;Save the current process
        mov     t_ebx,ebx                       ;
        mov     t_ecx,ecx                       ;
        mov     t_edx,edx                       ;
        mov     t_esi,esi                       ;
        mov     t_edi,edi                       ;
        mov     t_ebp,ebp                       ;
        mov     t_ss,ss                         ;
        mov     t_gs,gs                         ;
        mov     t_fs,fs                         ;
        mov     t_es,es                         ;
        pop     t_ds                            ;
        pop     t_eip                           ;
        pop     t_cs                            ;
        pop     t_flags                         ;
        mov     t_esp,esp                       ;

        mov     edi,_MLTICurrentProcess         ;...into the TST
        shl     edi,7                           ;
        add     edi,offset TST                  ;
        mov     esi,offset t_eax                ;
        mov     ecx,TST_SAVE_SIZE/4             ;
        cld                                     ;
        rep     movsd                           ;

MI_NextProcess:
        inc     _MLTICurrentProcess             ;Next process in the TST
        mov     eax,_MLTICurrentProcess         ;
        cmp     eax,_MLTIProcessCount           ;
        jl      MI_NoWrap                       ;
        mov     _MLTICurrentProcess,0           ;

MI_NoWrap:
        mov     esi,_MLTICurrentProcess         ;Restore new process from TST
        shl     esi,7                           ;
        add     esi,offset TST                  ;

        cmp     _MLTICurrentProcess,0           ;Make sure process 0 not halted
        je      MI_Active                       ;

        cmp     dword ptr [esi+S_ACTIVE],0      ;Is this process active?
        je      MI_NextProcess                  ;  (if not, next process)

MI_Active:
        mov     edi,offset t_eax                ;
        mov     ecx,TST_SIZE/4                  ;
        rep     movsd                           ;

        mov     eax,t_priority                  ;Setup new priority
        mov     CurrentProcessPriority,eax      ;

        mov     esp,t_esp                       ;Load up all the new regs
        push    t_flags                         ;
        mov     eax,t_eax                       ;
        mov     ebx,t_ebx                       ;
        mov     ecx,t_ecx                       ;
        mov     edx,t_edx                       ;
        mov     esi,t_esi                       ;
        mov     edi,t_edi                       ;
        mov     ebp,t_ebp                       ;
        mov     ss,t_ss                         ;
        push    t_cs                            ;
        push    t_eip                           ;
        mov     gs,t_gs                         ;
        mov     fs,t_fs                         ;
        mov     es,t_es                         ;
        mov     ds,t_ds                         ;

        sti                                     ;Laters (starts running new
        iretd                                   ;  process)

MLTIInterrupt_          EndP

        End

;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
;±   [Multia.asm  ] - End Of File                                           ±
;±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
