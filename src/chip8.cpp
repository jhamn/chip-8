#include "chip8.h"
#include <fstream>
#include <iostream>

Chip8::Chip8() {
    pc = 0x200;
    opcode = 0;
    I = 0;
    sp = 0;
    delay_timer = 0;
    sound_timer = 0;
    drawFlag = false;

    for(int i = 0; i < 4096; ++i) memory[i] = 0;
    for(int i = 0; i < 16; ++i) V[i] = 0;
    for(int i = 0; i < 16; ++i) stack[i] = 0;
    for(int i = 0; i < 64*32; ++i) gfx[i] = 0;
    for(int i = 0; i < 16; ++i) key[i] = 0;

    static const uint8_t chip8_fontset[80] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

for (int i = 0; i < 80; ++i)
    memory[i] = chip8_fontset[i];
}

void Chip8::loadROM(const std::string &filename) {
    std::ifstream rom(filename, std::ios::binary | std::ios::ate);
    if(!rom) {
        std::cerr << "Failed to open ROM\n";
        return;
    }
    std::streamsize size = rom.tellg();
    rom.seekg(0, std::ios::beg);
    if(size > (4096 - 512)) {
        std::cerr << "ROM too large to fit in memory\n";
        return;
    }
    rom.read((char*)(memory + 0x200), size);
}

void Chip8::emulateCycle() {
    // Fetch opcode (2 bytes)
    opcode = memory[pc] << 8 | memory[pc + 1];

    // Extract common parts
    uint16_t nnn = opcode & 0x0FFF;
    uint8_t  nn  = opcode & 0x00FF;
    uint8_t  n   = opcode & 0x000F;
    uint8_t  x   = (opcode & 0x0F00) >> 8;
    uint8_t  y   = (opcode & 0x00F0) >> 4;

    bool advancePC = true;

    switch (opcode & 0xF000) {
    case 0x0000:
        switch (nn) {
        case 0xE0: // CLS
            std::fill(gfx, gfx + 64*32, 0);
            drawFlag = true;
            break;
        
        case 0xEE: // RET
            sp--;
            pc = stack[sp];
            advancePC = false;
            break;

        default:
            std::cout << "Unknown opcode: " << std::hex << opcode << "\n";
            break;
        }
        break;

    case 0x1000: // JP addr
        pc = nnn;
        break;

    case 0x2000: // 2NNN - CALL addr
        stack[sp] = pc + 2;
        sp++;
        pc = nnn;
        advancePC = false;
        break;

    case 0x3000: // SE Vx, byte
        if (V[x] == nn) pc += 2;
        break;

    case 0x4000: // SNE Vx, byte
        if (V[x] != nn) pc += 2;
        break;

    case 0x5000: // SE Vx, Vy
        if (V[x] == V[y]) pc += 2;
        break;

    case 0x6000: // LD Vx, byte
        V[x] = nn;
        break;

    case 0x7000: // ADD Vx, byte
        V[x] += nn;
        break;

    case 0x8000:
        switch (n) {
        case 0x0: V[x]  = V[y]; break;
        case 0x1: V[x] |= V[y]; break;
        case 0x2: V[x] &= V[y]; break;
        case 0x3: V[x] ^= V[y]; break;

        case 0x4: { // ADD with carry
            uint16_t sum = V[x] + V[y];
            V[0xF] = sum > 255;
            V[x] = sum & 0xFF;    
        } break;

        case 0x5: // SUB Vx -= Vy
            V[0xF] = V[x] > V[y];
            V[x] -= V[y];
            break;

        case 0x6: // SHR Vx
            V[0xF] = V[x] & 1;
            V[x] >>= 1;
            break;

        case 0x7: // SUBN Vx = Vy - Vx
            V[0xF] = V[y] > V[x];
            V[x] = V[y] - V[x];
            break;

        case 0xE: // SHL Vx
            V[0xF] = (V[x] & 0x80) >> 7;
            V[x] <<= 1;
            break;

        default:
            std::cout << "Unknown opcode: " << std::hex << opcode << "\n";
            break;
        }
        break;

    case 0x9000: // 9XY0 - SNE Vx != Vy
        if (n == 0 && V[x] != V[y]) pc += 2;
        break;

    case 0xA000: // LD I, addr
        I = nnn;
        break;

    case 0xB000: // BNNN - JP V0 + addr
        pc = nnn + V[0];
        advancePC = false;
        break;

    case 0xC000: // CXNN - RND Vx, NN
        V[x] = (rand() & 0xFF) & nn;
        break;

    case 0xD000: { // DXYN - DRAW
        uint8_t xPos = V[x] % 64;
        uint8_t yPos = V[y] % 32;
        V[0xF] = 0;

        for(int row = 0; row < n; row++) {
            uint8_t pixel = memory[I + row];
            for(int col = 0; col < 8; col++) {
                if (pixel & (0x80 >> col)) {
                    int idx = (yPos + row) % 32 * 64 + (xPos + col) % 64;
                    if (gfx[idx] == 1) V[0xF] = 1;
                    gfx[idx] ^= 1;
                }
            }
        }
        drawFlag = true;
    } break;

    case 0xE000:
        switch(nn) {
        case 0x9E: // SKP Vx
            if (key[V[x]]) pc += 2;
            break;

        case 0xA1: // SKNP Vx
            if (!key[V[x]]) pc += 2;
            break;
        
        default:
            std::cout << "Unknown opcode: " << std::hex << opcode << "\n";
            break;
        }
        break;

    case 0xF000:
        switch(nn) {
        case 0x07: V[x] = delay_timer; break;
        case 0x0A: // LD Vx, K
            for(int i = 0; i < 16; i++){
                if(key[i]) {
                    V[x] = i;
                    goto keyPressDone;
                }
            }
            return;
        keyPressDone:
            break;

        case 0x15: delay_timer = V[x]; break;
        case 0x18: sound_timer = V[x]; break;
        case 0x1E: I += V[x]; break;
        case 0x29: I = V[x] * 5; break;
        case 0x33:
            memory[I]     =  V[x] / 100;
            memory[I + 1] = (V[x] / 10) % 10;
            memory[I + 2] =  V[x] % 10;
            break;

        case 0x55:
            for(int i = 0; i <= x; i++) memory[I + i] = V[i];
            break;

        case 0x65:
            for(int i = 0; i <= x; i++) V[i] = memory[I + i];
            break;

        default:
            std::cout << "Unknown opcode: " << std::hex << opcode << "\n";
        }
        break;

    default:
        std::cout << "Unknown opcode: " << std::hex << opcode << "\n";
        break;
    }
    if (advancePC) pc += 2;
}

void Chip8::updateTimers() {
    if(delay_timer > 0) --delay_timer;
    if(sound_timer > 0) --sound_timer;
}