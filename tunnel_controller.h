typedef enum {NORTH, SOUTH} direction_t;
direction_t opposite(direction_t dir);

void enter_tunnel(direction_t dir);
void exit_tunnel(direction_t dir);

