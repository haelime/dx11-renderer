# CLAUDE.md

C++/Direct3D 11 렌더러를 직접 만들며 공부하는 6개월 프로젝트 (2026-10-12 ~ 2027-04-11). 목표·단계·규칙은 [README.md](README.md), 학습 목록과 완료 기준은 [docs/learning.md](docs/learning.md), 최종 목표는 [docs/goals.md](docs/goals.md).

## Claude가 지켜야 할 것

- **코드를 대신 쓰지 않는다.** 사용자가 직접 친다. Claude는 개념 설명, 질문 상대, 디버깅 힌트, 설정·구조 조언만 한다. 소스 코드(.cpp/.h/.hlsl)를 작성하거나 고치지 않는다. 사용자가 명시적으로 요청한 비코드 작업(솔루션 파일, git, 폴더 정리 등)은 해도 된다.
- 설명은 한국어로.
- 범위를 넓히는 제안은 `docs/learning.md`의 "나중에" 목록으로 보낸다. 지금 단계를 먼저 끝내게 한다.
- `docs/devlog.md`, README "빌드" 섹션은 사용자가 직접 채운다.
- 커밋은 사용자가 요청할 때만.

## 환경과 결정 사항 (2026-10-11)

- **IDE**: Visual Studio 2026 (v18). 솔루션은 `dx11-renderer.slnx` (새 XML 형식).
- **SDK**: Windows SDK 10.0.26100.0 (D3D11, DXGI, d3dcompiler, DirectXMath 포함).
- **June 2010 DirectX SDK는 쓰지 않는다.** D3DX가 직접 만들 부분을 대신해 버리고, Windows SDK 헤더와 섞이면 충돌한다. 옛 튜토리얼의 `D3DX11*`, `D3DXMATRIX`, `XNAMath`는 `D3DCompileFromFile`, `DirectXMath`, 직접 구현한 텍스처 로딩(나중에 DirectXTex)으로 바꿔 읽는다.
- **저장소**: https://github.com/haelime/dx11-renderer (public, 브랜치 `main`).
- 소프트웨어 래스터라이저는 메인이 아님. DX11에서는 래스터라이저가 GPU 고정 기능이라 설정(RasterizerState)만 한다. 짧은 사이드 프로젝트(tinyrenderer)로 할지는 사용자가 결정.

## 솔루션 구성 계획

처음부터 다 만들지 않고 필요할 때 추가한다.

1. `Bench` (콘솔): 0단계 벤치마크 (sizeof/alignof, 배열 vs 연결 리스트, AoS vs SoA, memcpy 대역폭)
2. `Client` (실행 파일): Win32 창, 게임 루프 → 1단계 렌더링
3. `Renderer` (정적 lib): 1단계 중반에 Client에서 D3D 코드를 떼어 냄
4. `Common` (정적 lib) + `Server` (콘솔): 네트워크 단계

폴더: `src/`, `shaders/`, `assets/`, `docs/`. 나누면 `src/<프로젝트명>/`.

## 프로젝트 공통 설정

- 템플릿: 빈 프로젝트
- 플랫폼: x64만 (Win32/x86 구성 삭제)
- C++20, 유니코드 문자 집합
- 출력: `$(SolutionDir)bin\$(Platform)\$(Configuration)\`
- 중간: `$(SolutionDir)obj\$(ProjectName)\$(Platform)\$(Configuration)\`
- 디버깅 작업 디렉터리: `$(SolutionDir)` (shaders/assets 상대 경로용)
- 링크: `d3d11.lib`, `dxgi.lib`, `d3dcompiler.lib` (삼각형 단계부터)
- `.hlsl`을 프로젝트에 넣으면 VS가 FXC로 빌드하려 한다. 파일별로 셰이더 형식/모델(5.0)을 지정하거나, 런타임 컴파일이면 "빌드에서 제외".
- 디버그 레이어 쓰려면 Windows 선택적 기능 "그래픽 도구" 필요.

## 현재 상태와 다음 할 일

- [x] 솔루션 생성, 문서 정리, git init, GitHub 연결
- [x] 첫 커밋 & 푸시
- [x] 남은 `C:\Users\lorem\projects\render-practice` 폴더 삭제 (이미 없었음, 2026-10-11 확인)
- [ ] README "빌드" 섹션, devlog 오늘 기록 (사용자 직접)
- [ ] 10-12부터 0단계: `Bench` 프로젝트 추가 → sizeof/alignof 실험부터

0단계 진행 방식: 이론을 먼저 읽지 말고 벤치마크를 먼저 돌려 "왜 이렇게 나왔지?"를 만든 뒤 설명하려고 공부한다.
1주차 벤치마크, 2주차 이론(가상 메모리, PCI-e, GPU 구조) + 글, 3주차 Win32 창.

벤치마크 함정: Release로 잴 것 / 결과를 안 쓰면 최적화로 루프가 사라짐 / 반복 측정 / 연결 리스트 노드 순서를 섞을 것.

참고 자료: CS:APP 6·9장, Drepper "What Every Programmer Should Know About Memory", Microsoft Learn "Win32 및 C++ 시작", Fabian Giesen "A trip through the Graphics Pipeline 2011".
