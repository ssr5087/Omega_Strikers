# 역할
너는 언리얼 엔진 게임 개발자야

## 목표
오메가 스트라이커스 모작게임을 만들고 있어
Asher의 Special 스킬을 사용하는데 지금 오류가 나고있어
Asher_Special_Shield가 나오고나서 이런거 같아

## 해야할 일
1. 여기서 stack overflow가 일어나고 있어
2. UnrealEditor_Omega_Strikers!APlayerBase::ApplyKnockback() [C:\Unreal Projects\Omega_Strikers\Omega_Strikers\Source\Omega_Strikers\Private\PlayerBase.cpp:378]
   UnrealEditor_Omega_Strikers!APlayerBase::ReceiveImpact_Implementation() [C:\Unreal Projects\Omega_Strikers\Omega_Strikers\Source\Omega_Strikers\Private\PlayerBase.cpp:358]
   UnrealEditor_Omega_Strikers!IOSImpactReceiver::execReceiveImpact() [C:\Unreal Projects\Omega_Strikers\Omega_Strikers\Intermediate\Build\Win64\UnrealEditor\Inc\Omega_Strikers\UHT\OSImpactReceiver.gen.cpp:127]
3. PlayerBase에서도 충돌이 나는거같아
4. 한번 확인하고 수정해줘
5. 다하고 한글로 코드리뷰해줘

## 봐야할 파일
Asher라는 이름이 들아간 모든 파일을 사용하면 돼