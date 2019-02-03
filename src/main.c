#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "typedefs.h"
#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define modifier 10
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32

int display_width  = SCREEN_WIDTH * modifier;
int display_height = SCREEN_HEIGHT * modifier;

u8 chip8_fontset[80] = {
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
    0xF0, 0x80, 0xF0, 0x80, 0x80 // F
};

u8 drawFlag;

u16 opcode;
u8  memory[4096];
u8  V[16];
u16 I;
u16 pc;
u8  gfx[64 * 32];
u8  delay_timer;
u8  sound_timer;
u16 stack[16];
u16 sp;
u8  keys[16];

void loadGame(char* filename)
{
    info("Loading game: %s", filename);

    // Open file
    FILE* pFile = fopen(filename, "rb");
    if (pFile == NULL) {
        error("File error");
    }

    // Check file size
    fseek(pFile, 0, SEEK_END);
    long lSize = ftell(pFile);
    rewind(pFile);
    info("Filesize: %d\n", (int)lSize);

    // Allocate memory to contain the whole file
    char* buffer = (char*)malloc(sizeof(char) * lSize);

    // Copy the file into the buffer
    size_t result = fread(buffer, 1, lSize, pFile);
    if (result != lSize) {
        error("Reading error");
    }

    // Copy buffer to Chip8 memory
    if ((4096 - 512) > lSize) {
        for (int i = 0; i < lSize; ++i) {
            memory[i + 512] = buffer[i];
        }
    } else
        error("Error: ROM too big for memory");

    // Close file, free buffer
    fclose(pFile);
    free(buffer);
}

void initilize()
{
    // Initialize registers and memory once
    info("initilizing..");

    pc     = 0x200; // Program counter starts at 0x200 (Start adress program)
    opcode = 0; // Reset current opcode
    I      = 0; // Reset index register
    sp     = 0; // Reset stack pointer

    // Clear display
    for (int i = 0; i < 2048; ++i)
        gfx[i] = 0;

    // Clear stack
    for (int i = 0; i < 16; ++i)
        stack[i] = 0;

    for (int i = 0; i < 16; ++i)
        keys[i] = V[i] = 0;

    // Clear memory
    for (int i = 0; i < 4096; ++i)
        memory[i] = 0;

    // Load fontset
    for (int i = 0; i < 80; ++i)
        memory[i] = chip8_fontset[i];

    // Reset timers
    delay_timer = 0;
    sound_timer = 0;

    // Clear screen once
    drawFlag = true;

    srand(time(NULL));
}
void setKeys() {}

void drawPixel(int x, int y)
{
    glBegin(GL_QUADS);
    glVertex3f((x * modifier) + 0.0f, (y * modifier) + 0.0f, 0.0f);
    glVertex3f((x * modifier) + 0.0f, (y * modifier) + modifier, 0.0f);
    glVertex3f((x * modifier) + modifier, (y * modifier) + modifier, 0.0f);
    glVertex3f((x * modifier) + modifier, (y * modifier) + 0.0f, 0.0f);
    glEnd();
}

void drawGraphics()
{
    // Draw
    for (int y = 0; y < 32; ++y)
        for (int x = 0; x < 64; ++x) {
            if (gfx[(y * 64) + x] == 0)
                glColor3f(0.0f, 0.0f, 0.0f);
            else
                glColor3f(1.0f, 1.0f, 1.0f);
            drawPixel(x, y);
        }
}
void emulateCycle()
{
    // Fetch opcode
    opcode = memory[pc] << 8 | memory[pc + 1];

    // Decode opcode
    switch (opcode & 0xF000) {

    case 0x0000:
        switch (opcode & 0x000F) {
        case 0x0000: // 0x00E0: Clears the screen
            for (int i = 0; i < 2048; ++i)
                gfx[i] = 0x0;
            drawFlag = true;
            pc += 2;
            break;

        case 0x000E: // 0x00EE: Returns from subroutine
            --sp; // 16 levels of stack, decrease stack pointer to prevent overwrite
            pc = stack[sp]; // Put the stored return address from the stack back into the program counter
            pc += 2; // Don't forget to increase the program counter!
            break;

        default: printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
        }
        break;

    case 0x1000: // 0x1NNN: Jumps to address NNN
        pc = opcode & 0x0FFF;
        break;

    case 0x2000: // 0x2NNN: Calls subroutine at NNN.
        stack[sp] = pc; // Store current address in stack
        ++sp; // Increment stack pointer
        pc = opcode & 0x0FFF; // Set the program counter to the address at NNN
        break;

    case 0x3000: // 0x3XNN: Skips the next instruction if VX equals NN
        if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
            pc += 4;
        else
            pc += 2;
        break;

    case 0x4000: // 0x4XNN: Skips the next instruction if VX doesn't equal NN
        if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
            pc += 4;
        else
            pc += 2;
        break;

    case 0x5000: // 0x5XY0: Skips the next instruction if VX equals VY.
        if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
            pc += 4;
        else
            pc += 2;
        break;

    case 0x6000: // 0x6XNN: Sets VX to NN.
        V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
        pc += 2;
        break;

    case 0x7000: // 0x7XNN: Adds NN to VX.
        V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
        pc += 2;
        break;

    case 0x8000:
        switch (opcode & 0x000F) {
        case 0x0000: // 0x8XY0: Sets VX to the value of VY
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0001: // 0x8XY1: Sets VX to "VX OR VY"
            V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0002: // 0x8XY2: Sets VX to "VX AND VY"
            V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0003: // 0x8XY3: Sets VX to "VX XOR VY"
            V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0004: // 0x8XY4: Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't
            if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
                V[0xF] = 1; // carry
            else
                V[0xF] = 0;
            V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0005: // 0x8XY5: VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't
            if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
                V[0xF] = 0; // there is a borrow
            else
                V[0xF] = 1;
            V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
            pc += 2;
            break;

        case 0x0006: // 0x8XY6: Shifts VX right by one. VF is set to the value of the least significant bit of VX before
                     // the shift
            V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
            V[(opcode & 0x0F00) >> 8] >>= 1;
            pc += 2;
            break;

        case 0x0007: // 0x8XY7: Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't
            if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4]) // VY-VX
                V[0xF] = 0; // there is a borrow
            else
                V[0xF] = 1;
            V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;

        case 0x000E: // 0x8XYE: Shifts VX left by one. VF is set to the value of the most significant bit of VX before
                     // the shift
            V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
            V[(opcode & 0x0F00) >> 8] <<= 1;
            pc += 2;
            break;

        default: printf("Unknown opcode [0x8000]: 0x%X\n", opcode);
        }
        break;

    case 0x9000: // 0x9XY0: Skips the next instruction if VX doesn't equal VY
        if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
            pc += 4;
        else
            pc += 2;
        break;

    case 0xA000: // ANNN: Sets I to the address NNN
        I = opcode & 0x0FFF;
        pc += 2;
        break;

    case 0xB000: // BNNN: Jumps to the address NNN plus V0
        pc = (opcode & 0x0FFF) + V[0];
        break;

    case 0xC000: // CXNN: Sets VX to a random number and NN
        V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
        pc += 2;
        break;

    case 0xD000: // DXYN: Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
                 // Each row of 8 pixels is read as bit-coded starting from memory location I;
                 // I value doesn't change after the execution of this instruction.
                 // VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn,
                 // and to 0 if that doesn't happen
    {
        unsigned short x      = V[(opcode & 0x0F00) >> 8];
        unsigned short y      = V[(opcode & 0x00F0) >> 4];
        unsigned short height = opcode & 0x000F;
        unsigned short pixel;

        V[0xF] = 0;
        for (int yline = 0; yline < height; yline++) {
            pixel = memory[I + yline];
            for (int xline = 0; xline < 8; xline++) {
                if ((pixel & (0x80 >> xline)) != 0) {
                    if (gfx[(x + xline + ((y + yline) * 64))] == 1) {
                        V[0xF] = 1;
                    }
                    gfx[x + xline + ((y + yline) * 64)] ^= 1;
                }
            }
        }

        drawFlag = true;
        pc += 2;
    } break;

    case 0xE000:
        switch (opcode & 0x00FF) {
        case 0x009E: // EX9E: Skips the next instruction if the key stored in VX is pressed
            if (keys[V[(opcode & 0x0F00) >> 8]] != 0)
                pc += 4;
            else
                pc += 2;
            break;

        case 0x00A1: // EXA1: Skips the next instruction if the key stored in VX isn't pressed
            if (keys[V[(opcode & 0x0F00) >> 8]] == 0)
                pc += 4;
            else
                pc += 2;
            break;

        default: printf("Unknown opcode [0xE000]: 0x%X\n", opcode);
        }
        break;

    case 0xF000:
        switch (opcode & 0x00FF) {
        case 0x0007: // FX07: Sets VX to the value of the delay timer
            V[(opcode & 0x0F00) >> 8] = delay_timer;
            pc += 2;
            break;

        case 0x000A: // FX0A: A key press is awaited, and then stored in VX
        {
            bool keyPress = false;

            for (int i = 0; i < 16; ++i) {
                if (keys[i] != 0) {
                    V[(opcode & 0x0F00) >> 8] = i;
                    keyPress                  = true;
                }
            }

            // If we didn't received a keypress, skip this cycle and try again.
            if (!keyPress) return;

            pc += 2;
        } break;

        case 0x0015: // FX15: Sets the delay timer to VX
            delay_timer = V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;

        case 0x0018: // FX18: Sets the sound timer to VX
            sound_timer = V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;

        case 0x001E: // FX1E: Adds VX to I
            if (I + V[(opcode & 0x0F00) >> 8]
                > 0xFFF) // VF is set to 1 when range overflow (I+VX>0xFFF), and 0 when there isn't.
                V[0xF] = 1;
            else
                V[0xF] = 0;
            I += V[(opcode & 0x0F00) >> 8];
            pc += 2;
            break;

        case 0x0029: // FX29: Sets I to the location of the sprite for the character in VX. Characters 0-F (in
                     // hexadecimal) are represented by a 4x5 font
            I = V[(opcode & 0x0F00) >> 8] * 0x5;
            pc += 2;
            break;

        case 0x0033: // FX33: Stores the Binary-coded decimal representation of VX at the addresses I, I plus 1, and I
                     // plus 2
            memory[I]     = V[(opcode & 0x0F00) >> 8] / 100;
            memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
            memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
            pc += 2;
            break;

        case 0x0055: // FX55: Stores V0 to VX in memory starting at address I
            for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
                memory[I + i] = V[i];

            // On the original interpreter, when the operation is done, I = I + X + 1.
            I += ((opcode & 0x0F00) >> 8) + 1;
            pc += 2;
            break;

        case 0x0065: // FX65: Fills V0 to VX with values from memory starting at address I
            for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
                V[i] = memory[I + i];

            // On the original interpreter, when the operation is done, I = I + X + 1.
            I += ((opcode & 0x0F00) >> 8) + 1;
            pc += 2;
            break;

        default: printf("Unknown opcode [0xF000]: 0x%X\n", opcode);
        }
        break;

    default: warning("Unknown opcode: 0x%X\n", opcode);
    }

    // Update timers
    if (delay_timer > 0) --delay_timer;

    if (sound_timer > 0) {
        if (sound_timer == 1) warning("BEEP!\n");
        --sound_timer;
    }
}

void key_callback(GLFWwindow* window, s32 key, s32 scancode, s32 action, s32 mods)
{
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_ESCAPE: exit(0);
        case GLFW_KEY_1: keys[0x1] = 1; break;
        case GLFW_KEY_2: keys[0x2] = 1; break;
        case GLFW_KEY_3: keys[0x3] = 1; break;
        case GLFW_KEY_4: keys[0xC] = 1; break;
        case GLFW_KEY_Q: keys[0x4] = 1; break;
        case GLFW_KEY_W: keys[0x5] = 1; break;
        case GLFW_KEY_E: keys[0x6] = 1; break;
        case GLFW_KEY_R: keys[0xD] = 1; break;
        case GLFW_KEY_A: keys[0x7] = 1; break;
        case GLFW_KEY_S: keys[0x8] = 1; break;
        case GLFW_KEY_D: keys[0x9] = 1; break;
        case GLFW_KEY_F: keys[0xE] = 1; break;
        case GLFW_KEY_Z: keys[0xA] = 1; break;
        case GLFW_KEY_X: keys[0x0] = 1; break;
        case GLFW_KEY_C: keys[0xB] = 1; break;
        case GLFW_KEY_V: keys[0xF] = 1; break;
        }
    } else if (action == GLFW_RELEASE) {
        switch (key) {
        case GLFW_KEY_1: keys[0x1] = 0; break;
        case GLFW_KEY_2: keys[0x2] = 0; break;
        case GLFW_KEY_3: keys[0x3] = 0; break;
        case GLFW_KEY_4: keys[0xC] = 0; break;
        case GLFW_KEY_Q: keys[0x4] = 0; break;
        case GLFW_KEY_W: keys[0x5] = 0; break;
        case GLFW_KEY_E: keys[0x6] = 0; break;
        case GLFW_KEY_R: keys[0xD] = 0; break;
        case GLFW_KEY_A: keys[0x7] = 0; break;
        case GLFW_KEY_S: keys[0x8] = 0; break;
        case GLFW_KEY_D: keys[0x9] = 0; break;
        case GLFW_KEY_F: keys[0xE] = 0; break;
        case GLFW_KEY_Z: keys[0xA] = 0; break;
        case GLFW_KEY_X: keys[0x0] = 0; break;
        case GLFW_KEY_C: keys[0xB] = 0; break;
        case GLFW_KEY_V: keys[0xF] = 0; break;
        }
    }
}

int main(int argc, char** argv)
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    GLFWwindow* context = glfwCreateWindow(display_width, display_height, "CHIP-8", NULL, NULL);

    glewInit();

    glfwMakeContextCurrent(context);

    glfwSetKeyCallback(context, &key_callback);

    glLoadIdentity();
    glOrtho(0, display_width, display_height, 0, -1, 1);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    initilize();
    loadGame(argv[1]);

    static double limitFPS = 1.0 / 600.0;

    double lastTime = glfwGetTime(), timer = lastTime;
    double deltaTime = 0, nowTime = 0;

    while (!glfwWindowShouldClose(context)) {

        // - Measure time
        nowTime = glfwGetTime();
        deltaTime += (nowTime - lastTime) / limitFPS;
        lastTime = nowTime;

        // - Only update at 60 frames / s
        while (deltaTime >= 1.0) {
            deltaTime--;

            glfwPollEvents();

            // Emulate one cycle
            emulateCycle();

            // If the draw flag is set, update the screen
            if (drawFlag) {
                glClear(GL_COLOR_BUFFER_BIT);
                drawGraphics();
                glfwSwapBuffers(context);
                drawFlag = false;
            }
        }
    }

    return 0;
}
