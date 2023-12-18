#include "..\..\includes.hpp"

struct Local_data
{
	bool visualize_lag = false;

	c_baseplayeranimationstate* prediction_animstate = nullptr;
	c_baseplayeranimationstate* animstate = nullptr;

	Vector stored_real_angles = ZERO;
	Vector real_angles = ZERO;
	Vector fake_angles = ZERO;
};

class local_animations : public singleton <local_animations>
{
	bool real_server_update = false;
	bool fake_server_update = false;

	float real_simulation_time = 355.0f;
	float fake_simulation_time = 355.0f;

	bool random_roll_mem_value;

	CBaseHandle* handle = nullptr;

	float spawntime = 0.0f;
	float tickcount = 355.0f;

	float abs_angles = 0.0f;
	float abs_rolls = 0.0f;
	float value_pitch_speed = 0.0f;
	float pose_parameter[24];
	AnimationLayer layers[15];

	void update_fake_animations();
	void build_local_bones(player_t* local);
	void update_local_animations(c_baseplayeranimationstate* animstate);
public:
	Local_data local_data;

	void run(ClientFrameStage_t stage);
};
