from headers import math
import random

visualize = False

try:
    import pygame
    visualize = True
except ImportError:
    ...

window_width = 800
window_height = 600

def main():
    random.seed(1337)
    width = 0
    totalblocks = 0
    running = True
    total = 0

    try:
        width = int(input("input the x axis value >>"))
    except Exception as e:
        print(e)
        input()
        return
    
    if width < 1 or width > 128:
        raise ValueError('ValueError (too big or too small)')
        return

    totalblocks = math.CalculateTotalBlocks(width)
    print(f"Total blocks {totalblocks}")
    print(f"Total stacks {math.CalculateTotalStacks(totalblocks)}")
    print(f"Total stacks int {int(math.CalculateTotalStacks(totalblocks))}")

    if visualize:
        pygame.init()
        screen = pygame.display.set_mode((window_width, window_height))
        pygame.display.set_caption("output")

        while running:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False

            screen.fill((30,30,30)) # draws background color

            offset = 0
            side = width
            while side > 0:
                pygame.draw.rect(screen,(255, ((side * 73) % 256), 0),((window_width / 2) - (width*4 /2) + offset, (window_height / 2) - (width*4 /2) + offset, (width*4) - (offset*2),(width*4) - (offset*2))) # draws rect using predetermined random color value and adjusting for offset based on loop iter
                # math :)
                total = total + (side * 4) - 12
                side -= 1
                offset += 4

            pygame.display.flip()       
    return

main()