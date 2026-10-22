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

1. `Client` (실행 파일, `src/Client/`): Win32 창, 게임 루프. 하위 시스템 "창", 진입점 `wWinMain`
2. `Renderer` (정적 lib, `src/Renderer/`): D3D 코드. 처음부터 분리함 (2026-10-11). Client가 프로젝트 참조로 링크
3. `Common` (정적 lib) + `Server` (콘솔): 네트워크 단계
4. (나중에) `Bench` (콘솔): 벤치마크. learning.md "나중에" 참고

폴더: `src/<프로젝트명>/`, `shaders/`, `assets/`, `docs/`. 참고용 샘플은 저장소 밖 `C:\Users\lorem\projects\dx11-sdk-samples-reworked` (walbourn/directx-sdk-samples-reworked, 옛 이름 directx-sdk-samples. D3DX 없는 버전. 정답지로만 본다). `microsoft/DirectX-SDK-Samples`는 D3DX를 NuGet으로 쓰므로 보지 않는다.
프로젝트 폴더를 옮길 땐 VS를 닫고 옮긴 뒤 slnx 경로를 고친다 (열린 채로 옮기면 VS가 옛 경로에 다시 저장함).

## 프로젝트 공통 설정

- 템플릿: 빈 프로젝트
- 플랫폼: x64만 (Win32/x86 구성 삭제)
- C++20, 유니코드 문자 집합
- 출력: `$(SolutionDir)bin\$(Platform)\$(Configuration)\`
- 중간: `$(SolutionDir)obj\$(ProjectName)\$(Platform)\$(Configuration)\`
- 디버깅 작업 디렉터리: `$(SolutionDir)` (shaders/assets 상대 경로용)
- 링크: `d3d11.lib`, `d3dcompiler.lib`는 `Renderer.cpp`의 `#pragma comment`로 Renderer가 직접 챙긴다 (Client 링커 설정에는 없음)
- 그 밖: `/W4`, `/MP`, `WIN32_LEAN_AND_MEAN`, `NOMINMAX`, 포함 디렉터리 `$(SolutionDir)src\` (두 프로젝트 공통). 작업 디렉터리는 `.vcxproj`에 있음 (`.user` 아님)
- 인코딩: 소스는 UTF-8 (`/utf-8`, 루트 `.editorconfig`). 셰이더는 BOM 없이 (fxc가 BOM을 거부할 수 있음)
- 이름: 헝가리안 표기 (이 프로젝트만, 2026-10-21 결정. GitHub 리뷰 때 타입이 보이게). 범위 `g_`/`m_`/`s_` + 타입 `p`(ComPtr 포함)/`h`/`sz`/`b`/`n`·`i`/`f`·`d`/`u`·`dw`. 새 이름에 `lp`는 쓰지 않고 `p`로. Win32 매개변수 이름은 문서 그대로. 타입을 바꾸면 이름도 같이 바꾼다
- D3D 객체는 `Microsoft::WRL::ComPtr`로 들고, `Release()`를 직접 부르지 않는다. 멤버는 선언 역순으로 해제되므로 디바이스를 맨 위에 선언
- `.hlsl`을 프로젝트에 넣으면 VS가 FXC로 빌드하려 한다. 파일별로 셰이더 형식/모델(5.0)을 지정하거나, 런타임 컴파일이면 "빌드에서 제외".
- 디버그 레이어 쓰려면 Windows 선택적 기능 "그래픽 도구" 필요.

## 현재 상태와 다음 할 일

- [x] 솔루션 생성, 문서 정리, git init, GitHub 연결
- [x] 첫 커밋 & 푸시
- [x] 남은 `C:\Users\lorem\projects\render-practice` 폴더 삭제 (이미 없었음, 2026-10-11 확인)
- [x] 0단계(벤치마크) 건너뛰기로 결정, 1단계를 10-12부터 시작 (2026-10-12)
- [x] Client(exe) + Renderer(정적 lib) 생성, `src/`로 이동, `/utf-8`, 하위 시스템 "창" + `wWinMain` → 창 만들기 ⓪ 진입점 확인(MessageBox) 완료, 커밋·푸시
- [x] 창 만들기 1~5단계, `PeekMessage` 게임 루프
- [x] 1-1: 디바이스·스왑 체인·RTV·뷰포트·Clear·Present, 디버그 레이어 → 파란 화면
- [x] 1-2: 셰이더(`shaders/triangle_VS.hlsl`, `_PS.hlsl`, 런타임 컴파일), 입력 레이아웃, 정점 버퍼 → 색 보간 삼각형 → 인덱스 버퍼 사각형 (인덱스 `UINT` + `R32_UINT`)
- [x] 설정 정리: x64만, `bin\`/`obj\`, `/W4` 등 (2026-10-20)
- [x] D3D 코드를 `Renderer` 클래스(정적 lib)로 분리, `InitDevice(HWND)`/`Render()`, 소멸자에서 정리, ComPtr 적용 (2026-10-21)
- [x] 1-3 앞부분: `Timer`(QPC), `Camera`(View/Projection), 상수 버퍼 + WVP(전치), 회전하는 큐브(8정점·36인덱스, 바깥에서 시계 방향), 깊이 버퍼(D24S8 + DSV) + 큐브 두 개 (2026-10-22)
- [ ] **다음: 1-3 나머지** Update/Render 분리(시간은 Render 밖, 물체마다 World) → FPS 카메라(키보드·마우스 입력, yaw/pitch, 외적으로 right, 델타 타임)
- [ ] 미뤄 둔 실험: 어느 GPU로 그리는지 확인(`adapter->GetDesc()`, 이 PC는 AMD 내장 + RTX 5060 Ti), `ReportLiveDeviceObjects`로 누수 확인, 직접 만든 `MyComPtr`(선택)
- [ ] README "빌드" 섹션, devlog 기록 (사용자 직접)

진행 방식: 단계를 작게 나눠 단계마다 실행, 실행 전에 결과를 예측한다. 처음엔 문서·샘플을 보고 쳐도 되지만 한 줄씩 뜻을 말할 수 있을 때만 넘어가고, 다음 날 안 보고 다시 친다. 이해한 줄을 일부러 빼 보며 확인한다.

참고 자료: Microsoft Learn "Win32 및 C++ 시작" 모듈 1, 함수별 Learn 문서(구문/매개 변수/반환 값/설명/요구 사항), `Tutorial01.cpp`(막힐 때만).
