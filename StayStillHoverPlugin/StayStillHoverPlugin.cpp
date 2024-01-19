#include "pch.h"
#include "StayStillHoverPlugin.h"
#include <algorithm>

#define PI 3.14159265
#define UPDATES_PER_SECOND 120

#define MAX(a, b) ((a > b) ? a : b)
#define MIN(a, b) ((a < b) ? a : b)


BAKKESMOD_PLUGIN(StayStillHoverPlugin, "Makes the car hover in the air. Can enable target orientation to practice air control.", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;


void StayStillHoverPlugin::onLoad()
{
	_globalCvarManager = cvarManager;
	//cvarManager->log("Plugin loaded!");


	cvarManager->registerCvar("stay_still_hover_enabled", "0", "Enable StayStillHoverPlugin");
	cvarManager->registerCvar("stay_still_hover_target_orientation", "0", "Enable Target Orientation");
	cvarManager->registerCvar("stay_still_hover_target_rolls", "1", "Enable Target Rolling When You Roll");

	gameWrapper->HookEvent("Function TAGame.Car_TA.SetVehicleInput", [this](std::string eventName) { //called every physics tick.
		tick();
	});

	gameWrapper->HookEvent("Function GameEvent_TA.Countdown.BeginState", [this](std::string eventName) { //beginning of countdown, or reset in freeplay
		freeplayReset();
	});


	gameWrapper->RegisterDrawable([this](CanvasWrapper canvas) {
		render(canvas);
	});
}

void StayStillHoverPlugin::onUnload()
{
}

bool StayStillHoverPlugin::isEnabled()
{
	CVarWrapper enableCvar = cvarManager->getCvar("stay_still_hover_enabled");
	if (!enableCvar) return false;
	return enableCvar.getBoolValue();
}

bool StayStillHoverPlugin::isTargetOrientationEnabled()
{
	CVarWrapper targetOrientationCvar = cvarManager->getCvar("stay_still_hover_target_orientation");
	if (!targetOrientationCvar) return false;
	return targetOrientationCvar.getBoolValue();
}

bool StayStillHoverPlugin::isTargetRollingEnabled()
{
	CVarWrapper targetRollsCvar = cvarManager->getCvar("stay_still_hover_target_rolls");
	if (!targetRollsCvar) return false;
	return targetRollsCvar.getBoolValue();
}

void StayStillHoverPlugin::tick()
{
	if (removeBotsBool) {
		removeBots();
		removeBotsBool = false;
	}

	if (!isEnabled()) return;
	if (!gameWrapper->IsInFreeplay()) return;

	ServerWrapper server = gameWrapper->GetCurrentGameState();
	if (!server) return;

	CarWrapper car = gameWrapper->GetLocalCar();
	BallWrapper ball = server.GetBall();
	if (!car || !ball) return;

	ball.SetLocation(Vector{ 0, 4400, ball.GetRadius() });
	ball.SetVelocity(Vector{ 0, 0, 0 });

	car.SetLocation(Vector{0, 0, 600});
	car.SetVelocity(Vector{ 0, 0, 0 });

	dampenNoInputAngularMomentum(car);


	if (isTargetOrientationEnabled())
	{
		updateTargetOrientation(server, car);
	}
	else
	{
		removeBots();
	}
}

void StayStillHoverPlugin::dampenNoInputAngularMomentum(CarWrapper car)
{
	if (!car) return;
	ControllerInput input = car.GetInput();
	float dampenFactor = 0.96f;
	//dampenFactor = 1.02f; //makes so it doesn't slow down at all

	if (noAngularMomentumInputActive(input))
	{
		Vector v = car.GetAngularVelocity();

		v.X *= dampenFactor;
		v.Y *= dampenFactor;
		v.Z *= dampenFactor;

		car.SetAngularVelocity(v, false);
	}
}

bool StayStillHoverPlugin::noAngularMomentumInputActive(ControllerInput input)
{
	return input.Pitch == 0 && input.Roll == 0 && input.Yaw == 0;
}

void StayStillHoverPlugin::updateTargetOrientation(ServerWrapper server, CarWrapper car)
{
	CarWrapper target = getEmptyNamedCar(server);
	if (!target) {
		spawnCar();
		return;
	}

	if (isTargetRollingEnabled()) {
		ControllerInput ownInput = car.GetInput();
		ControllerInput input = target.GetInput();
		input.Roll = ownInput.Roll;
		target.SetInput(input);
	}

	target.SetLocation(Vector{ 0, 200, 750 });
	target.SetVelocity(Vector{ 0, 0, 0 });

	//LOG("Timer: {}", timer);

	if (isOrientationCloseEnough(car.GetRotation(), target.GetRotation())) {
		closeEnoughCounter++;
		insideTarget = true;
	} else {
		closeEnoughCounter--;
		closeEnoughCounter = MAX(0, closeEnoughCounter);
		insideTarget = false;
	}
	

	if (timer > UPDATES_PER_SECOND * timeToHitTargetS || closeEnoughCounter > UPDATES_PER_SECOND) {
		timer = 0;
		orientTarget(target);

		resultTimer = UPDATES_PER_SECOND;

		if (closeEnoughCounter > UPDATES_PER_SECOND) {
			result = true;
			score++;
		} else {
			result = false;
			score = 0;
		}
		closeEnoughCounter = 0;
	}

	timer++;
	resultTimer--;
}

void StayStillHoverPlugin::orientTarget(CarWrapper car)
{
	Rotator previous = car.GetRotation();
	Rotator newRotation = getRandomOrientation();
	while (angleBetweenVectors(RotatorToVector(previous), RotatorToVector(newRotation)) < 45) {
		newRotation = getRandomOrientation();
	}
	car.SetRotation(newRotation);
	car.SetAngularVelocity(Vector{ 0, 0, 0 }, false);
}

Rotator StayStillHoverPlugin::getRandomOrientation()
{
	return VectorToRotator(Vector{ (float)(rand() % 180) - 90, (float)(rand() % 360) - 180, (float)(rand() % 360) - 180 });
}

bool StayStillHoverPlugin::isOrientationCloseEnough(Rotator orientation, Rotator target)
{
	float angle = angleBetweenVectors(RotatorToVector(orientation), RotatorToVector(target));
	return angle <= targetAngleThreshold;
}

float StayStillHoverPlugin::angleBetweenVectors(Vector first, Vector second)
{
	float dot = Vector::dot(first, second);

	return acosf(dot / (first.magnitude() * second.magnitude())) * (180.0 / PI);
}

/*Vector StayStillHoverPlugin::RotatorToVector(Rotator r)
{
	int x = (int)(r.Pitch * 90.0 / 16384);
	int y = (int)(r.Yaw * 180.0 / 32768);
	int z = (int)(r.Roll * 180.0 / 32768);

	return Vector{ x, y, z };
}*/

void StayStillHoverPlugin::spawnCar()
{
	if (!isEnabled()) return;
	if (!gameWrapper->IsInFreeplay()) return;

	ServerWrapper server = gameWrapper->GetCurrentGameState();
	if (!server) return;

	CarWrapper car = gameWrapper->GetLocalCar();
	if (!car) return;

	if (server.GetCars().Count() >= 2) {
		return;
	}

	server.SpawnCar(car.GetLoadoutBody(), "botti");
}

//TODO: figure out a better way to differentiate between all the cars
CarWrapper StayStillHoverPlugin::getEmptyNamedCar(ServerWrapper server) {
	ArrayWrapper<CarWrapper> cars = server.GetCars();
	for (int i = 0; i < cars.Count(); i++)
	{
		if (cars.Get(i).GetOwnerName().empty()) {
			return cars.Get(i);
		}
	}
	return NULL;
}

void StayStillHoverPlugin::removeBots()
{
	if (!gameWrapper->IsInFreeplay()) return;
	ServerWrapper server = gameWrapper->GetCurrentGameState();
	if (!server) return;
	
	ArrayWrapper<CarWrapper> cars = server.GetCars();
	if (cars.Count() - 1 == 0) {
		return;
	}
	LOG("Deleting bots, bot count: {}", cars.Count() - 1);

	for (int i = 0; i < cars.Count(); i++)
	{
		CarWrapper c = cars.Get(i);

		if (c.GetOwnerName().empty() || c.GetOwnerName() == "botti") {
			server.RemoveCar(c);
			server.RemovePlayer(c.GetAIController());
			c.Destroy();
		}
	}
}

float StayStillHoverPlugin::getPercentDone()
{
	return MIN(100, closeEnoughCounter / timeToHoldTargetS);
}

void StayStillHoverPlugin::freeplayReset() {
	if (!gameWrapper->IsInFreeplay()) return;
	gameWrapper->SetTimeout([this](GameWrapper* game) {
		removeBots();
	}, 0.07f);
}

void StayStillHoverPlugin::render(CanvasWrapper canvas)
{
	if (!gameWrapper->IsInFreeplay()) return;
	if (!isEnabled()) return;
	if (!isTargetOrientationEnabled()) return;

	LinearColor black = LinearColor{ 0, 0, 0, 255 };
	LinearColor darkGray = LinearColor{ 50, 50, 50, 220 };
	LinearColor lightGray = LinearColor{ 150, 150, 150, 150 };
	LinearColor green = LinearColor{ 0, 255, 0, 255 };
	LinearColor red = LinearColor{ 255, 0, 0, 255 };

	int width = canvas.GetSize().X;
	int height = canvas.GetSize().Y;

	Vector2F insideTextPos = Vector2F{ (float)(width - 700), 150 };
	canvas.SetPosition(insideTextPos);

	canvas.SetColor(darkGray);
	canvas.FillBox(Vector2F{ 650, 150});
	canvas.SetPosition(insideTextPos);

	if (insideTarget)
	{
		canvas.SetColor(green);
		canvas.DrawString("INSIDE!", 10.0, 10.0, false);
	}
	else
	{
		canvas.SetColor(red);
		canvas.DrawString("OUTSIDE!", 10.0, 10.0, false);
	}

	int percentDone = (int) getPercentDone();

	canvas.SetPosition(Vector2F{ (float)(width / 2) - 50, (float)(height - 50) });
	canvas.SetColor(green);
	canvas.DrawString(std::to_string(percentDone) + "%", 3.0, 3.0, false);

	if (resultTimer > 0) {
		Vector2F resultTextPosition = Vector2F{ (float)(150), 200 };
		canvas.SetPosition(resultTextPosition);
		canvas.SetColor(darkGray);
		if (result) {
			canvas.FillBox(Vector2F{ 335, 75 });
			canvas.SetPosition(resultTextPosition);
			canvas.SetColor(green);
			canvas.DrawString("SUCCESS!", 5.0, 5.0, false);
		} else {
			canvas.FillBox(Vector2F{ 200, 75 });
			canvas.SetPosition(resultTextPosition);
			canvas.SetColor(red);
			canvas.DrawString("FAIL!", 5.0, 5.0, false);
		}
	}

	Vector2F scorePos = insideTextPos + Vector2F{0, 200};

	canvas.SetPosition(scorePos);
	canvas.SetColor(lightGray);
	canvas.FillBox(Vector2F{ 335, 75 });
	canvas.SetPosition(scorePos);
	canvas.SetColor(black);
	canvas.DrawString("Score: " + std::to_string(score), 5.0, 5.0, false);
}