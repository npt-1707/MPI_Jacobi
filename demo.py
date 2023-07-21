import pygame
import numpy as np
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("--path",
                    type=str,
                    default="demo/parallel_result_12_12_0.0100.txt")
args = parser.parse_args()


# Load matrix states from the txt file
def load_matrix_states(file_path, N):
    with open(file_path, 'r') as file:
        content = file.readlines()
    states = [
        np.array(list(map(float,
                          row.strip().split('\t')))) for row in content
    ]
    return np.array(states).reshape(-1, N, N)


# Initialize Pygame
pygame.init()

# Load matrix states from the txt file
file_path = args.path
N = int(file_path.split('_')[2])  # Change this to the size of your N*N matrix
print("Loading ...")
matrix_states = load_matrix_states(file_path, N)
num_states = matrix_states.shape[0]
max_value = np.max(matrix_states)
min_value = np.min(matrix_states)

# Calculate the display size for each element rectangle and screen size
element_display_size = int(800 / N)
screen_width = element_display_size * N + 100
screen_height = element_display_size * N + 50

# Colors
black = (0, 0, 0)
white = (255, 255, 255)
grey = (128, 128, 128)
# Custom low and high colors for scaling
low_color = (0, 0, 255)  # Customize these colors as needed
high_color = (255, 0, 0)

# Initialize Pygame window
screen = pygame.display.set_mode((screen_width, screen_height))
pygame.display.set_caption('Matrix Heatmap')

# Initialize state index and progress
current_state = 0
progress = 0

progess_width = screen_width - 100

legend_width = 100
legend_height = element_display_size * N


def scale_color(value, low_color, high_color, min_value, max_value):
    scaled_value = (value - min_value) / (max_value - min_value)
    scaled_value = scaled_value**0.6
    r = int(low_color[0] + scaled_value * (high_color[0] - low_color[0]))
    g = int(low_color[1] + scaled_value * (high_color[1] - low_color[1]))
    b = int(low_color[2] + scaled_value * (high_color[2] - low_color[2]))
    return (r, g, b)


print("Calculating ...")
heatmap_cache = np.zeros((num_states, N, N, 3), dtype=np.uint8)
for i in range(num_states):
    heatmap_cache[i] = np.array([
        scale_color(value, low_color, high_color, min_value, max_value)
        for value in matrix_states[i].flatten()
    ]).reshape(N, N, 3)

print("Running ...")
running = True
dragging = False  # To keep track of dragging state

while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        elif event.type == pygame.KEYDOWN:
            # Handle arrow key presses
            if event.key == pygame.K_LEFT:
                current_state = max(0, current_state - 1)
            elif event.key == pygame.K_RIGHT:
                current_state = min(num_states - 1, current_state + 1)
        elif event.type == pygame.MOUSEBUTTONDOWN:
            if event.button == 1:  # Left mouse button
                # Check if the progress bar is clicked
                mouse_x, mouse_y = event.pos
                if mouse_x in range(
                        50, 50 + progess_width + 1) and mouse_y in range(
                            screen_height - 50, screen_height):
                    dragging = True
        elif event.type == pygame.MOUSEBUTTONUP:
            if event.button == 1:  # Left mouse button
                dragging = False
        elif event.type == pygame.MOUSEMOTION:
            if dragging:
                # Update progress based on mouse position
                mouse_x = event.pos[0]
                progress = max(0, min(mouse_x / progess_width, 1))
                current_state = int(progress * (num_states - 1))

    # Clear the screen
    screen.fill(grey)

    for y in range(N):
        for x in range(N):
            color = heatmap_cache[current_state][x][y]
            pygame.draw.rect(
                screen, color,
                (x * element_display_size, y * element_display_size,
                 element_display_size, element_display_size))

    # Draw white grid lines
    for x in range(N + 1):
        pygame.draw.line(screen, grey, (x * element_display_size, 0),
                         (x * element_display_size, element_display_size * N))
    for y in range(N + 1):
        pygame.draw.line(screen, grey, (0, y * element_display_size),
                         (element_display_size * N, y * element_display_size))

    # Draw progress bar with label
    # pygame.draw.rect(screen, black,
    #                  (0, element_display_size * N, screen_width, 100))
    pygame.draw.rect(screen, white,
                     (50, element_display_size * N + 15,
                      int(progess_width * current_state / num_states), 20))
    font = pygame.font.Font(None, 20)
    label = font.render(f'{current_state + 1}/{num_states}', True, white)
    screen.blit(label, (10, element_display_size * N + 10))

    # Draw legend bar with annotated values
    # pygame.draw.rect(
    #     screen, black,
    #     (screen_width - legend_width, 0, legend_width, legend_height))
    for i in range(N):
        scaled_value = i / (N - 1)
        color = scale_color(scaled_value, high_color, low_color, 0, 1)
        pygame.draw.rect(
            screen, color,
            (screen_width - legend_width + 20, i * element_display_size,
             element_display_size, element_display_size))

    # Draw legend labels
    font = pygame.font.Font(None, 20)
    min_label = font.render(f'{np.min(matrix_states):.2f}', True, white)
    max_label = font.render(f'{np.max(matrix_states):.2f}', True, white)
    screen.blit(min_label,
                (screen_width - legend_width + 30,
                 element_display_size * N - element_display_size // 2))
    screen.blit(max_label,
                (screen_width - legend_width + 30, element_display_size // 2))

    # Update the display
    pygame.display.flip()

# Quit Pygame
pygame.quit()
