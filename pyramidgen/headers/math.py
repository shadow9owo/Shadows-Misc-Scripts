def CalculateTotalBlocks(x):
    total = 0
    side = x
    while side > 0:
        total = total + (side * 4) - 12
        side = side - 1
    return total

def CalculateTotalStacks(blocks):
    return blocks / 64
