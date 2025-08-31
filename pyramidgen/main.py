from headers import math

visualize = False

try:
    import pygame
    visualize = True
except ImportError:
    ...

def main():
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
        screen = pygame.display.set_mode((800, 600))
        pygame.display.set_caption("output")

        while running:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False

            screen.fill((30,30,30))

            offset = 0
            side = width
            while side > 0:
                pygame.draw.rect(screen, (255, min(255, offset*20), 0), 
                                 (offset, offset, side * 4 - offset, side * 4 - offset))
                total = total + (side * 4) - 12
                side -= 1
                offset += 4

            pygame.display.flip()       
    return

main()