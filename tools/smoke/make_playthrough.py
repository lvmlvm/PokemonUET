#!/usr/bin/env python3
"""Generates the smoke-test replay script (see include/replay.h for the format).

Walks a fresh game through: title -> new game -> character select -> E3 -> Student
Button Room (get Pokemon) -> G2 -> first challenge room -> battle (fight, switch,
forced switches) -> menu (party, save, high scores) -> quit. Routes are computed with
BFS over the collision layers of the shipped .map files, so they follow map edits.

Scenario "continue" relaunches on the save the new-game playthrough left behind: Continue,
then check the menu's high scores and quit.

Usage: make_playthrough.py <assets dir> <screenshot dir> [new-game|continue]  > script.txt
"""
import sys
from collections import deque
from pathlib import Path

SCREEN_TILES_X, SCREEN_TILES_Y = 13, 11      # 832x704 window, 64px tiles
PLAYER_OFFSET_X, PLAYER_OFFSET_Y = 6, 5      # player is drawn at camera tile + (6, 5)
STEP_FRAMES = 24                              # one tile at walking speed is 16 frames
WARP_FRAMES = 60                              # fade out + reload + fade in
KEY_FOR = {(0, -1): "W", (0, 1): "S", (-1, 0): "A", (1, 0): "D"}


class GameMap:
    def __init__(self, path, has_overlay, extra_blocked=()):
        tokens = path.read_text().split()
        self.id, self.height, self.width = int(tokens[0]), int(tokens[1]), int(tokens[2])
        cells = self.height * self.width
        pos = 3 + cells + 1                    # skip tile layer and its end flag
        flat = [int(t) for t in tokens[pos:pos + cells]]
        self.collision = [flat[r * self.width:(r + 1) * self.width] for r in range(self.height)]
        lines = path.read_text().splitlines()
        # NPCs, warps and interactive tiles all block movement.
        for i, line in enumerate(lines):
            if line in ("MAP_NEXT_NPC", "WARP_NEXT_TILE", "INTER_NEXT_TILE"):
                x, y = (int(v) for v in lines[i + 1].split()[:2])
                self.collision[y][x] = 1
        for x, y in extra_blocked:
            self.collision[y][x] = 1

    def walkable(self, x, y):
        # The camera must stay inside the map, which limits where the player can stand.
        if not (PLAYER_OFFSET_X <= x <= self.width - (SCREEN_TILES_X - PLAYER_OFFSET_X)):
            return False
        if not (PLAYER_OFFSET_Y <= y <= self.height - (SCREEN_TILES_Y - PLAYER_OFFSET_Y)):
            return False
        return self.collision[y][x] == 0

    def route(self, start, goal):
        prev = {start: None}
        queue = deque([start])
        while queue:
            cur = queue.popleft()
            if cur == goal:
                break
            for dx, dy in KEY_FOR:
                nxt = (cur[0] + dx, cur[1] + dy)
                if nxt not in prev and self.walkable(*nxt):
                    prev[nxt] = cur
                    queue.append(nxt)
        if goal not in prev:
            raise SystemExit(f"map {self.id}: no route {start} -> {goal}")
        path = []
        while goal != start:
            path.append(goal)
            goal = prev[goal]
        return list(reversed(path))


class Script:
    def __init__(self, shots):
        self.frame = 0
        self.lines = []
        self.shots = shots
        self.pos = None

    def at(self, text, wait):
        self.lines.append(f"{self.frame} {text}")
        self.frame += wait

    def comment(self, text):
        self.lines.append(f"# {text}")

    def tap(self, key, wait=20):
        self.at(f"tap {key}", wait)

    def click(self, x, y, wait=20):
        self.at(f"click {x} {y}", wait)

    def shot(self, name):
        self.at(f"shot {self.shots}/{name}.bmp", 1)

    def walk(self, game_map, goal):
        for step in game_map.route(self.pos, goal):
            self.tap(KEY_FOR[(step[0] - self.pos[0], step[1] - self.pos[1])], STEP_FRAMES)
            self.pos = step

    def warp(self, key, arrive):
        self.tap(key, WARP_FRAMES)
        self.pos = arrive


def continue_game(shots):
    s = Script(shots)
    s.comment("title screen with a save file: wait for the intro animation, then Continue")
    s.frame = 330
    s.shot("20_title_with_save")
    s.click(416, 406, 90)                   # Continue
    s.shot("21_continued")
    s.tap("V", 30)
    s.click(620, 300, 30)                   # High Score
    s.shot("22_highscores_after_restart")
    s.click(640, 490, 30)
    s.tap("Escape", 30)
    s.at("quit", 60)
    return s


def main():
    assets, shots = Path(sys.argv[1]), sys.argv[2]
    if len(sys.argv) > 3 and sys.argv[3] == "continue":
        print("\n".join(continue_game(shots).lines))
        return

    maps = assets / "map"
    e3 = GameMap(maps / "e3.map", False)
    e3i = GameMap(maps / "e3i.map", True)
    button_room = GameMap(maps / "e3i_2.map", True)
    g2 = GameMap(maps / "g2.map", False)
    g2i = GameMap(maps / "g2i.map", True)
    chal5 = GameMap(maps / "chal5.map", False, extra_blocked=[(12, 8)])  # challenge-room gate NPC

    s = Script(shots)
    s.comment("title screen (no save file): wait for the intro animation, then New Game")
    s.frame = 330
    s.shot("01_title")
    s.click(416, 434, 60)
    s.shot("02_setup")
    s.click(230, 380, 90)                   # Ruby
    s.shot("03_overworld")

    s.comment("E3 exterior -> E3 interior -> Student Button Room")
    s.pos = (20, 10)
    s.warp("W", (29, 17))
    s.walk(e3i, (31, 9))
    s.warp("W", (13, 22))
    s.walk(button_room, (13, 17))
    s.tap("W")                              # face the Pokemon giver
    for _ in range(5):
        s.tap("X", 30)
    s.shot("04_got_pokemon")

    s.comment("menu: party view, save, high scores")
    s.tap("V", 30)
    s.click(620, 110, 30)                   # Pokemon
    s.shot("05_menu_party")
    s.click(640, 490, 30)                   # back
    s.click(620, 300, 30)                   # High Score
    s.shot("06_menu_highscores")
    s.click(640, 490, 30)                   # back
    s.click(620, 205, 30)                   # Save (closes the menu)

    s.comment("back out to E3 exterior, over to G2")
    s.walk(button_room, (13, 22))
    s.warp("S", (31, 9))
    s.walk(e3i, (29, 17))
    s.warp("S", (20, 10))
    s.walk(e3, (30, 21))
    s.warp("D", (7, 21))
    s.walk(g2, (32, 17))
    s.warp("W", (27, 34))
    s.shot("07_g2_interior")
    s.walk(g2i, (27, 9))
    s.warp("W", (12, 22))

    s.comment("first challenge room: talk to the trainer until the battle starts")
    s.walk(chal5, (12, 14))
    s.tap("W")
    for _ in range(6):
        s.tap("X", 30)
    s.frame += 150                          # map -> battle fade
    s.shot("08_battle_start")
    for _ in range(3):                      # intro dialogue
        s.tap("X", 90)

    s.comment("battle turns; the party-slot clicks only matter after a forced switch")
    for turn in range(14):
        s.click(415, 600, 30)               # Fight
        if turn == 0:
            s.shot("09_moves")
        s.click(172, 555, 30)               # first move
        for _ in range(7):
            s.tap("X", 80)
        for slot_y in (271, 349, 427):      # party slots in the forced-switch screen
            s.click(396, slot_y, 60)
        if turn == 2:
            s.shot("10_mid_battle")
            s.click(117, 602, 30)           # Pokemon button: voluntary switch
            s.shot("11_switch_screen")
            s.click(396, 349, 60)           # second slot
            for _ in range(3):
                s.tap("X", 80)
    s.frame += 200
    s.shot("12_after_battle")
    s.tap("V", 30)
    s.click(620, 300, 30)                   # High Score
    s.shot("13_highscores_after_battle")
    s.click(640, 490, 30)
    s.tap("Escape", 30)
    s.at("quit", 60)

    print("\n".join(s.lines))


if __name__ == "__main__":
    main()
