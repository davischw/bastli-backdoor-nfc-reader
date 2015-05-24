#include "opener.h"

int main(){
	config_struct config = config_struct();
	Opener opener(config, 2 * 60);
	opener.open_to("basld");
	opener.display_ascii_art("Welcome Commander");
	opener.display_text("Yolo!");
	opener.start();
}