#include <stdio.h>

#include <common/common.h>
#include <common/error.h>
#include <logger/logger.h>
#include <common/map/map.h>
#include <common/vector/vector.h>
#include <ecs/ecs.h>
#include <ecs/system.h>

typedef struct {
	float x;
	float y;
} position_t;

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} color_t;

MAYBE_DEFINE_COMPONENT_TYPE(position_t)
MAYBE_DEFINE_COMPONENT_TYPE(color_t)

void test(maybe_system_t* system) {
	maybe_system_component_iterator_t iterator;
	color_t* color;
	uint32_t i = 2;

	maybe_system_init_component_iterator(system, MAYBE_COMPONENT_ID(color_t), &iterator);

	while (iterator.current_component_pointer) {
		color = (color_t*)iterator.current_component_pointer;	
		if (color->r == 0) color->r = 1;
		color->r *= i;

		//MAYBE_DEBUG_LOG("{0i}", color->r);

		maybe_system_component_iterator_get_next_component(system, &iterator);
		
		i++;
	}	
}

int application_init(void) {
	maybe_world_t world;
	maybe_entity_t entity;
	uint32_t i;
 
	MAYBE_INFO_LOG("Initializing application");

	maybe_world_init(&world);

	MAYBE_REGISTER_COMPONENT_TYPE(&world, position_t);
	MAYBE_REGISTER_COMPONENT_TYPE(&world, color_t);

	maybe_world_register_system(&world, &test, 1, MAYBE_COMPONENT_ID(color_t));

	maybe_world_add_entity(&world, 2, &entity, MAYBE_COMPONENT_ID(position_t), MAYBE_COMPONENT_ID(color_t));
	maybe_world_add_entity(&world, 2, &entity, MAYBE_COMPONENT_ID(position_t), MAYBE_COMPONENT_ID(color_t));

	for (i = 0; i < 10; i++) {
		maybe_world_update(&world);
	}

	maybe_world_free(&world);

	return 0;
}
