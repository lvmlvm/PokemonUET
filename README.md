# Pokémon UET
Formerly Pokémon VNU but that is only possible if LTNC is the only subject in the semester.

## Giới thiệu về Game:
<img width="960" alt="image" src="https://user-images.githubusercontent.com/29592868/169844629-622cf24d-a0f9-4ed0-bc52-dda1f0b21964.png">

- Pokémon UET là một game mang tính tái tạo lại một phần nhỏ của tựa game Pokémon Emerald của GAME FREAK trong ngôn ngữ lập trình C++ và SDL2.0.
- Những map của game được thiết kế lại để chúng nhìn giống 2 tòa nhà chính của Trường Đại học Công Nghệ, ĐHQGHN (nhà G2 và nhà E3).
- Nhà G2 đã được trưng dụng làm một cơ sở để người chơi chiến đấu Pokémon mới một loạt NPC trong Game.
- Nhà E3, dù vẫn mang tính chất của khu nhà E3 vốn có, giờ có mục tiêu chính là giúp người chơi thuê mượn Pokémon.
- Mục tiêu chính của game là sử dụng 3 con Pokémon của người chơi và battle qua nhiều phòng trong nhà G2 nhất có thể, sau mỗi battle, điểm số sẽ được tích lũy dần lên.
- Ngoài battle ra thì trong game cũng có những cái NPC và những object với những trao đổi và than thở... gì gì đó.


## Cách build và chạy game
Game dùng CMake và SDL2 (kèm SDL2_image, SDL2_ttf, SDL2_mixer), chạy được trên macOS, Linux và Windows.

### macOS
```sh
brew install cmake sdl2 sdl2_image sdl2_ttf sdl2_mixer
./run.sh
```

### Linux (Ubuntu/Debian)
```sh
sudo apt install cmake g++ libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev
./run.sh
```

### Windows
Cài [Visual Studio](https://visualstudio.microsoft.com/) (C++), CMake và [vcpkg](https://vcpkg.io/), rồi:
```bat
vcpkg install sdl2 sdl2-image sdl2-ttf sdl2-mixer[mpg123] --triplet x64-windows
run.bat -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
```

`run.sh` / `run.bat` build game vào thư mục `build/` rồi chạy. Có thể chạy lại trực tiếp bằng `build/pokemon_uet`.

File save được lưu trong thư mục dữ liệu của người dùng (macOS: `~/Library/Application Support/UET/PokemonUET/`, Windows: `%APPDATA%\UET\PokemonUET\`, Linux: `~/.local/share/UET/PokemonUET/`). File `data/player.sav` của phiên bản cũ sẽ được tự động chuyển sang khi mở game lần đầu.

## Dành cho lập trình viên
- `src/core/`: SDL, tài nguyên (asset), nhạc, hiệu ứng chuyển cảnh, vòng lặp game (`Game`) và giao diện `Scene`.
- `src/scenes/`: màn hình tiêu đề, chọn nhân vật, bản đồ (overworld) và trận đấu.
- `src/battle/`: luật chơi của trận đấu (`BattleEngine`) và dữ liệu Pokémon — không phụ thuộc SDL.
- `src/world/`: bản đồ, camera, NPC, người chơi. `src/save/`: đọc/ghi file save. `src/ui/`: nút bấm, chữ, menu.
- Unit test: `ctest --test-dir build` (sau khi build).
- Smoke test: `tools/smoke/run.sh` build bản có AddressSanitizer/UndefinedBehaviorSanitizer rồi tự chơi một ván theo kịch bản (game mới → nhận Pokémon → đấu trong G2 → menu → thoát → mở lại và Continue), lưu ảnh chụp màn hình vào `build-asan/smoke/`. Với `SMOKE_GOLDEN=<thư mục>`, ảnh được so sánh từng pixel với một lần chạy trước để kiểm tra refactor không làm thay đổi game.

## Phân công nhiệm vụ trong nhóm
- Ngô Danh Lam: Phát triển cấu trúc Pokémon và hệ thống battle; Xây dựng những hàm và cấu trúc cơ sở của game.
- Lê Vũ Minh: Phát triển engine map, camera, NPC, audio, các Menu và nút bấm trong game, xây dựng frontend của battle.
- Lê Huy Tuấn Anh: Xử lý đồ họa, nhập liệu cho game, phát triển Text, Dialogue.

## Instruction
![image](https://user-images.githubusercontent.com/29592868/169816056-b2d8eeec-f55c-4a95-84ab-6aee60790fa6.png)

## Game mechanic
### Speed
Mỗi lượt, Pokemon có chỉ số speed cao hơn sẽ được tấn công trước.
Nếu bằng nhau, người chơi được ưu tiên trước.
### Typing
Mỗi Pokemon sẽ mang 1 hoặc 2 hệ.
Mỗi một chiêu thức sẽ mang 1 hệ.
#### STAB
Same Type Attack Bonus:
  Khi hệ của Pokemon tấn công trùng với vệ của chiêu thức, sát thương sẽ được tăng 50%.
#### Type Effectiveness
Tương tác các hệ được cho bởi bảng sau



![image](https://user-images.githubusercontent.com/29592868/169819959-e4ae407b-d83b-4869-bb7a-b08ab931670c.png)

