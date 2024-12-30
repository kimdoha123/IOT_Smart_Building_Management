# 🏢 IoT Smart Building Management System

## 📌 프로젝트 개요

IoT Smart Building Management System은 ATmega128, STM32, Raspberry Pi, Arduino를 활용하여  
건물 내 환경 데이터를 모니터링하고 원격으로 제어하는 스마트 빌딩 관리 시스템입니다.  
센서와 액추에이터를 통해 실내 환경을 최적화하고 사용자 편의성을 향상시키는 것을 목표로 합니다.

---

## 👥 팀원

- **정상훈**  
- **김도하**  

---

## 🛠️ 개발 환경

### 📌 하드웨어 구성  
- **Client:** ATmega128  
- **Controller** STM32F411RE 
- **Server:** Raspberry Pi  
- **Sensor:** DHT11 (온습도 센서), CDS (조도 센서)  
- **Actuator:** 환풍기(Fan), 램프(LED), 창문(Step Motor), 창문(Servo Motor) 
- **Network:** Wi-Fi 모듈 

### 📌 소프트웨어 구성  
- **프로그래밍 언어:** C
- **개발 도구:** Arduino IDE, STM32CubeIDE
- **데이터베이스:** MariaDB  
- **네트워크 프로토콜:** MQTT, HTTP  

---

## ⚙️ 시스템 구조

### 📌 시스템 동작 원리  

1. **센서 데이터 수집:** DHT11과 CDS 센서를 통해 실내 환경 데이터 수집  
2. **데이터 처리 및 저장:** 수집된 데이터는 Raspberry Pi에서 처리되어 MariaDB에 저장  
3. **액추에이터 제어:** 환경 조건에 따라 팬, 램프, 블라인드, 창문을 자동 또는 수동 제어  
4. **사용자 인터페이스:** 모바일 앱이나 웹 인터페이스를 통해 실시간 모니터링 및 제어 가능  

---

## 고찰

### 느낀점
- 다양한 통신 방식을 통합하는 과정에서 블루투스와 와이파이의 장단점을 직접 경험하면서 상황별 기술의 선택의 중요성을 배움.
- 기능을 추가 및 변경이 필요했던 상황에서, 모듈화 설계가 유지보수와 확장에 있어 얼마나 중요한 알게 됨.

### 어려웠던 점
- Arduino의 Wi-Fi가 간헐적으로 끊기는 문제가 발생하여 원활한 프로젝ㄷ트 진행이 어려웠음.
- 네트워크에 다수의 사용자가 접속하면, 네트워크 부하로 인해 응답 속도가 느려지는 문제가 발생함.

## 향후 개발 방향
- 보안 강화
    - 암호화 통신 및 사용자 인증 도입
- 에너지 효율
    - AI기반 예측 모델을 통한 자동 제어 시스템 구현
- 스마트 학습 기능
    - 사용자의 패턴을 학습해 상황에 맞게 장치 제어
- 사용자 경험(UI/UX)
    - 여러 사용자가 동시 접속 및 컨트롤을 할 수 있도록 기능 추가
- 시스템 확장
    - 연기 감지기, 누수 감지기 등을 추가하여 안전성과 편의성을 높임
    - 다양한 장치를 간단하게 추가 할 수 있도록 모듈화
