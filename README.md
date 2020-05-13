# OS_Inverted_Page_Table

[CAUTION]

32bit 기반 우분투 16.04에서 실행시킬 수 있습니다. 또한, 연습용 OS인 SSUOS의 실제 메모리는 128MB이며 Page Frame 한개의 크기는 4KB입니다. 더 자세한 명세는 
pdf파일로 소스파일에 첨부하였습니다.

If you want to execute this source code on your computer, you have to run this software on 32bit Ubuntu Linux 16.04. In 
addition, The size of physical memory of the 'SSUOS' which is OS for practice is 128MB and the size of one page frame is 4KB.

I'm not a native English speaker,so please consider it when you read this script. :)

[COPYRIGHT]

이 프로젝트의 모든 저작권은 숭실대학교 운영체제 LAB에있습니다. 이 프로젝트는 컴퓨터학부 운영체제 수업의 5번째 프로젝트였습니다.

All copyright is belong to Soongsil University OS Lab and this project was fifth subject of OS class.

[OVERVIEW]

본 프로젝트의 주요 목표는 크게 3가지입니다. 첫 번째는 Page Allocator를 이해하는 것이고 두 번째는 Level Hash를 구현하는 것 그리고 세 번째는 Inverted Page
Table을 구현하는 것입니다. 과제를 수행하기 위해서 실습자는 가상 메모리, 페이징, 역 페이지 테이블 그리고 Level Hash에 대한 이해를 필요로합니다. 만약 실습자가 기초
사항을 이해했다면, 위에서 정의한 2가지를 구현해야합니다. 구현해야할 사항은 [OBJECTS]에 명시했습니다.

The main objects of this project have three parts. First of all, we need to understand 'Page Allocator'. Second of all, we 
have to make 'Level Hash'. Last, we should make an 'Inverted Page Table'. In order to make this project, we need to comprehend
virtual memory, paging method, inverted page table, and level hash. If you understood these things, you have to make two
things that we have defined. I wrote detail to be made at [OBJECTS].

- 가상 메모리(Virtual Memory)

가상 메모리 기법은 디스크의 일부를 확장된 RAM처럼 사용할 수 있도록 해주는 기법입니다. 이 기법은 당장 사용하지 않는 메모리 블록을 디스크에 올림으로써 사용 가능한 메모
리 공간을 늘립니다. 만약 디스크에 저장된 메모리 블록이 필요해지면 다시 RAM에 적재되며 다른 블록이 디스크에 저장됩니다. 연습용 OS인 SSUOS는 가상메모리 주소를 Page
Table과 Page Directory를 통해 맵핑하여 실제 메모리 주소를 사용합니다. RAM 공간 초과로 인한 디스크 SWAP영역은 SSUOS에선 사용하지 않습니다. (실제로 현재 사용
되는 수 많은 OS들은 디스크를 안쓰는 쪽으로 발전하고 있습니다.)

Virtual Memory method allows a portion of disk can be used as expanded RAM. This method can expand an usable memory space by
loading unusing memory block to DISK. If the memory block saved in DISK is needed, it will be loaded to RAM once again and the
other memory block will be loaded to DISK. 'SSUOS' uses a real memory address by mapping a virtual memory through 'Page Table'
and 'Page Directory'. SWAP area owing to RAM space expansion is not used in SSUOS. (Actually, many OS currently used are 
evolving towards like this.) 

- 페이징(Paging)

페이징이란, 가상 메모리를 모두 같은 크기의 블록으로 관리하는 기법을 의미합니다. 가상 메모리의 일정한 크기를 가진 블록을 Page라고 하고 실제 메모리의 일정한 크기를 가
진 블록을 Frame이라고 합니다. 따라서, 가상 메모리 구현이란 가상 메모리가 참조하는 Page 주소를 실제 메모리 Frame 주소로 변환하는 것을 의미합니다. 가상 메모리 주
소는 가상 메모리의 Page 주소를 Page Table을 통해 해당 Page에 해당하는 실제 메모리의 Frame에 접근하는 수단입니다. SSUOS의 Page 크기는 4KB이며 Page Table
과 Page Directory로 나뉘는 2단계 페이지 테이블 구조를 사용합니다.

Paging method is to manage the virtual memory into blocks of same size. A block with a certain size of virtual memory is 
called a page,and a block with a certain size of physical memory is called a frame. Therefore, making a virtual memory means 
that the OS should convert a page address referred by virtual memory to a frame address of physical memory. A virtual memory 
address is a means to access to physical memory frame through a page table referred by virtual memory page number. The size of
page in SSUOS is 4KB and SSUOS uses a two-level table structure splited into page table and page directory.

- 역페이지 테이블(Inverted Page Table)

많은 프로세스들이 생성되면 Page Table이 차지하는 공간이 커지는 문제를 해결하기 위해 고안된 기법입니다. 역페이지 테이블은 프로세스의 수나 지원되는 가상페이지의 수와는
관계없이 항상 주기억장치의 일정한 부분만을 사용합니다. 1개의 역페이지 테이블이 존재하며 하나의 frame은 그에 해당하는 하나의 entry만 가지고 있습니다. 가상주소에서 페
이지 번호에 해당하는 부분을 hash 함수를 통해 나온 특정 hash value를 페이지 인덱스 번호로 사용합니다. 여러 개의 가상주소가 한 entry를 가리킬 수 있으므로(오버플로
우) 그에 대비해 체이닝(chaining) hash를 사용합니다.

Inverted Page Table is a method to solve the problem that the space occupied by the page table increases when many processes 
are created. It always uses an only certain portion of main memory regardless of the number of virtual page. There is one 
inverted page table and one frame has only one entry corresponding to it. The part corresponding to the page number in the 
virtual address uses the specific hash value from the hash function as the page index number. It uses a chaining hash since 
multiple virtual addresses can point one entry.

- Level Hash

메모리 쓰기 성능을 향상시키기 위한 hash 기법입니다. Top Level과 Bottom Level의 2 level hash table을 이루며 Top Level Hash table은 Bottom Level
Hash Table의 2배 크기입니다. 2개의 Top Level은 1개의 Bottom Level을 공유하며 1개의 Level Bucket은 4개의 슬롯을 가집니다. 서로 다른 두 개의 Top 버킷
에 접근하기 위해 2개의 hash function이 존재합니다. 

Level Hash is a method to improve the memory writing performance. Level Hash consists of 2-level hash table of top level and 
bottom level and top level hash table is 2 times larger than bottom level hash table. Two top level share one bottom level and
one hash table has 4 slots. There are two hash functions to access two differenct top buckets.

[OBJECTS]

1. Level Hash 구현하기 : Make a level hash

2. Level Hash를 사용하는 역페이지 테이블 구현하기 : Make an Inverted Page Table using level hash
