# OS_Inverted_Page_Table

[COPYRIGHT]

이 프로젝트의 모든 저작권은 숭실대학교 운영체제 LAB에있습니다. 이 프로젝트는 컴퓨터학부 운영체제 수업의 5번째 프로젝트였습니다.

All copyright is belong to Soongsil University OS Lab and this project was fifth subject of OS class.

[OVERVIEW]

본 프로젝트의 주요 목표는 크게 3가지입니다. 첫 번째는 Page Allocator를 이해하는 것이고 두 번째는 Level Hash를 구현하는 것 그리고 세 번째는 Inverted Page
Table을 구현하는 것입니다. 과제를 수행하기 위해서 실습자는 가상 메모리, 페이징, 역 페이지 테이블 그리고 Level Hash에 대한 이해를 필요로합니다. 만약 실습자가 기초
사항을 이해했다면, 위에서 정의한 2가지를 구현해야합니다. 구현해야할 사항은 [OBJECTS]에 명시했습니다.

- 가상 메모리

메모리 사용량이 늘어남에 따라 디스크의 일부를 마치 확장된 RAM처럼 사용할 수 있도록 해주는 기법으로써 커널이 RAM에 적재된 당장 사용하지 않는 메모리 블록을 디스크에함
으로써 사용 가능한 메모리 공간을 늘립니다. 만약 디스크에 저장된 메모리 블록이 필요해지면 다시 RAM에 적재되며 다른 블록이 디스크에 저장됩니다. 연습용 OS인 SSUOS는
가상메모리 주소를 Page Table과 Page Directory를 통해 맵핑하여 실제 메모리 주소를 사용합니다. RAM 공간 초과로 인한 디스크 SWAP영역은 SSUOS에선 사용하지 않습
니다. (실제로 현재 사용되는 수 많은 OS들은 디스크를 안쓰는 쪽으로 발전하고 있습니다.)

- 페이징

페이징이란, 가상 메모리를 모두 같은 크기의 블록으로 관리하는 기법을 의미합니다. 가상 메모리의 일정한 크기를 가진 블록을 Page라고 하고 실제 메모리의 일정한 크기를 가
진 블록을 Frame이라고 합니다. 따라서, 가상 메모리 구현이란, 가상 메모리가 참조하는 Page 주소를 실제 메모리 Frame 주소로 변환하는 것을 의미합니다. 가상 메모리 주
소는 가상 메모리의 Page 주소를 Page Table을 통해 해당 Page에 해당하는 실제 메모리의 Frame에 접근하는 수단입니다. SSUOS의 Page 크기는 4KB이며 Page Table
과 Page Directory로 나뉘는 2단계 페이지 테이블 구조를 사용합니다.

- 역페이지 테이블

많은 프로세스들이 생성되면 Page Table이 차지하는 공간이 커지는 문제를 해결하기 위해 고안된 기법입니다. 역페이지 테이블은 프로세스의 수나 지원되는 가상페이지의 수와는
관계없이 항상 주기억장치의 일정한 부분만을 사용합니다. 1개의 역페이지 테이블이 존재하며 하나의 frame은 그에 해당하는 하나의 entry만 가지고 있습니다. 가상주소에서 페
이지 번호에 해당하는 부분을 hash 함수를 통해 나온 특정 hash value를 페이지 인덱스 번호로 사용합니다. 여러 개의 가상주소가 한 entry를 가리킬 수 있으므로(오버플로
우) 그에 대비해 체이닝(chaining) hash를 사용합니다.

- Level Hash

메모리 쓰기 성능을 향상시키기 위한 hash 기법입니다. Top Level과 Bottom Level의 2 level hash table을 이루며 Top Level Hash table은 Bottom Level
Hash Table의 2배 크기입니다. 2개의 Top Level은 1개의 Bottom Level을 공유하며 1개의 Level Bucket은 4개의 슬롯을 가집니다. 서로 다른 두 개의 Top 버킷
에 접근하기 위해 2개의 hash function이 존재합니다. 

32bit 기반 우분투 16.04에서 실행시킬 수 있습니다. 또한, 연습용 OS인 SSUOS의 실제 메모리는 128MB이며 Page Frame 한개의 크기는 4KB입니다. 더 자세한 명세는 
pdf파일로 소스파일에 첨부하였습니다.

If you want to execute this source code on your computer, you have to run this software on 32bit Ubuntu Linux 16.04. In 
addition, The size of physical memory of the 'SSUOS' which is OS for practice is 128MB and the size of one page frame is 4KB.

[OBJECTS]

1. Level Hash 구현하기

2. Level Hash를 사용하는 역페이지 테이블 구현하기
