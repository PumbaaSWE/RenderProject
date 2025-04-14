#include "application.h"


class Application1 : public tde::Application
{

	float time = 0;
	int secs = 0;
	bool showTime = false;



public:
	Application1()
	{
		appName = "Woop";
		//mvp = 
	}

	void Init() override {

	}


	void FixedUpdate(float dt) override {

	}


	void Update(float dt) override {


	}
	void Render(float dt, float extrapolation) override {

	}

};





int main()
{
	Application1 app;
	if (app.Create(720, 420) == tde::Success)
		app.Start();
	return 0;
}