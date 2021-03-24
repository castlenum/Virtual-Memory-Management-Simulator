< Virtual Memory Management Simulator >  
========================================
· Virtual Memory Systems의 one-level, two-level Page Table, Inverted Page Table system을 구현.   
· /mtraces: 프로그램에서 접근한 메모리 주소(Virtual address)가 순차적으로 기록된 memory trace가 존재.    

=trace file의 memory trace 포맷=    
**16진수로 표현한 32bit의 메모리 주소&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Read | Write**&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;· virtual address size 32bit (4GB)   
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;· page size 12bit (4KB) 

# 1) Virtual Memory Simulator 인자
<pre><code> memsim [-s] simType firstLevelBits PhysicalMemorySizeBits TraceFileNames...... </code></pre>   

> - [-s] &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;가상주소에서 변환된 물리주소를 출력하는 옵션.       
> - simType&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;실행할 Simulation Type을 지정하는 옵션.   
>> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;-0&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;FIFO, LRU 방식의 One-level page table system이 실행.    
>> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;-1&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Two-level page table system이 실행.    
>> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;-2&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Inverted page table system이실행.   
>> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;-3 이상&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;One-level, Two-level, Inverted Page table system을 차례로 실행.   
> - firstLevelBits&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Two-level page table system 실행 시 사용되는 인자.   
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;first level page table에 접근에 사용되는 메모리 주소의 bits 의 수.    
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;e.g) 인자 값이 8인 경우, 32bit 중 8bit가 first level page table로 사용됨.   
> - PhysicalMemorySizeBits Physical Memory의 크기를 나타내는 인자입니다.   
&nbsp;&nbsp;e.g) 인자 값이 n인 경우 Physical Memory의 크기는 2^n bytes.   
> - TraceFileNames, .... Trace File의 경로를 지정하는 가변 배열의 인자.   

# 2) Compile
<pre><code> gcc -o memsim memsimhw.c </code></pre>


# 3) 수행 예
<pre><code></code>$ ./memsim -s 1 10 32 ./mtraces/gcc.trace ./mtraces/bzip.trace </pre>
$ ./memsim –s 1 10 32 ./mtraces/gcc.trace ./mtraces/bzip.trace process 0 
opening ./mtraces/gcc.trace
process 1 opening ./mtraces/bzip.trace
Num of Frames 1048576 Physical Memory Size 4294967296 bytes ============================================================= The Two-Level Page Table Memory Simulation Starts ..... ============================================================= Two-Level procID 0 traceNumber 1 virtual addr 2f8773d8 pysical addr 3d8 
Two-Level procID 1 traceNumber 1 virtual addr 6645b58 pysical addr 1b58 
Two-Level procID 0 traceNumber 2 virtual addr 3d729358 pysical addr 2358 
Two-Level procID 1 traceNumber 2 virtual addr 6645b58 pysical addr 1b58
......
...... Two-Level procID 0 traceNumber 999999 virtual addr 2f8773e0 pysical addr 3ae3e0 
Two-Level procID 1 traceNumber 999999 virtual addr 6645ba0 pysical addr 723ba0 
Two-Level procID 0 traceNumber 1000000 virtual addr 3d729358 pysical addr 24358 
Two-Level procID 1 traceNumber 1000000 virtual addr 5fe5180 pysical addr 2eb180
**** ../mtraces/gcc.trace ***** Proc 0 
Num of traces 1000000
Proc 0 Num of second level page tables allocated 164 Proc 0 
Num of Page Faults 2852
Proc 0 Num of Page Hit 997148
**** ../mtraces/bzip.trace ***** Proc 1 
Num of traces 1000000
Proc 1 Num of second level page tables allocated 39 Proc 1 
Num of Page Faults 317
Proc 1 Num of Page Hit 999683
=Test Case=
memsim –s 0 10 20 ./mtraces/gcc.trace ./mtraces/bzip.trace ./mtraces/random0.trace
memsim –s 1 10 32 ./mtraces/bzip.trace
memsim –s 2 10 32 ./mtraces/gcc.trace ./mtraces/bzip.trace
memsim –s 3 10 24 ./mtraces/gcc.trace ./mtraces/bzip.trace ./mtraces/bzip.trace
memsim 3 10 18 ./mtraces/bzip.trace ./mtraces/gcc.trace ./mtraces/sixpack.trace
./mtraces/swim.trace ./mtraces/random0.trace ./mtraces/random2.trace
memsim 3 10 22 ./mtraces/bzip.trace ./mtraces/gcc.trace ./mtraces/sixpack.trace
./mtraces/swim.trace ./mtraces/random0.trace ./mtraces/random2.trace
memsim 3 7 19 ./mtraces/bzip.trace ./mtraces/gcc.trace ./mtraces/sixpack.trace ./mtraces/swim.trace./mtraces/random0.trace ./mtraces/random2.trace
memsim 3 9 20 ./mtraces/bzip.trace ./mtraces/gcc.trace ./mtraces/sixpack.trace
./mtraces/swim.trace
./mtraces/random0.trace ./mtraces/random2.trace ./mtraces/bzip.trace
./mtraces/gcc.trace
./mtraces/sixpack.trace ./mtraces/swim.trace ./mtraces/random0.trace ./mtraces/random2.trace 
memsim 3 8 21 ./mtraces/bzip.trace ./mtraces/gcc.trace ./mtraces/sixpack.trace
./mtraces/swim.trace
./mtraces/random0.trace ./mtraces/random2.trace ./mtraces/bzip.trace ./mtraces/gcc.trace
./mtraces/sixpack.trace ./mtraces/swim.trace ./mtraces/random0.trace ./mtraces/random2.trace
