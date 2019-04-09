namespace Motor{
	enum Direction{
		Backward = -1,
		Stop,
		Forward,
		Left,
		Right
	};
	struct WheelPin{
		int controller;
		int A;
		int B;
	};
	class Wheel{
	public:
		WheelPin pin;
		int compensate = 0;
		int speed = 0;

	public:
		Wheel(WheelPin wp){this -> pin = wp;};
		~Wheel(){};
		void setCompensate(int n){ compensate = n; };
		void setSpeed(int n){analogWrite(pin.controller, n + compensate);};
	};
}

class Car{
	public:
	class photoSensor{
		public:
			int pin;
			int blackValue;
			int whiteValue;

		public:
			photoSensor(){};
			~photoSensor(){};
			void setPin(int p){this -> pin = p;};
			void setBlackValue(int bk){this -> blackValue = bk;};
			void setWhiteValue(int wh){this -> whiteValue = wh;};
			int getPin(){return pin;};
			int read(){return analogRead(pin);};
			bool isBlack(){return read() <= whiteValue - 100;};
	}sensor[4];

	Motor::Wheel Left,Right;
	Motor::Direction carDirection;
	int carSpeed;
	const int StatusLightRed = 8;
	const int StatusLightYellow = 12;
	const int StatusLightGreen = 13;

	public:
	Car(Motor::WheelPin wheelPinL,Motor::WheelPin wheelPinR, int sensor[], int compensate,int speed):
	Left(wheelPinL), Right(wheelPinR)
	{
		for(int i = 0;i < 4;++i) this -> sensor[i].setPin(sensor[i]);

		this -> sensor[0].setBlackValue(470);
		this -> sensor[0].setWhiteValue(600);

		this -> sensor[1].setBlackValue(405);
		this -> sensor[1].setWhiteValue(900);

		this -> sensor[2].setBlackValue(570);
		this -> sensor[2].setWhiteValue(800);

		this -> sensor[3].setBlackValue(200);
		this -> sensor[3].setWhiteValue(400);

		if(compensate < 0){
			Left.setCompensate(0);
			Right.setCompensate(-compensate);
		}
		else{
			Left.setCompensate(compensate);
			Right.setCompensate(0);
		}

		setSpeed(speed);

	};
	~Car(){};
	void pinInit() {
		pinMode(Left.pin.controller,OUTPUT);
		pinMode(Left.pin.A,OUTPUT);
		pinMode(Left.pin.B,OUTPUT);

		pinMode(Right.pin.controller,OUTPUT);
		pinMode(Right.pin.A,OUTPUT);
		pinMode(Right.pin.B,OUTPUT);

		pinMode(StatusLightRed,OUTPUT);
		pinMode(StatusLightYellow,OUTPUT);
		pinMode(StatusLightGreen,OUTPUT);
	};
	void ActivateLineFollowing();
	void setDirection(Motor::Direction direction);
	void setSpeed(int n){
		carSpeed = n;
		Left.setSpeed(carSpeed);
		Right.setSpeed(carSpeed);
	};
	void rotate(Motor::Direction );
	void sensorTest(){
		Serial.print(sensor[0].read());
		Serial.print(" / ");
		Serial.print(sensor[1].read());
		Serial.print(" / ");
		Serial.print(sensor[2].read());
		Serial.print(" / ");
		Serial.println(sensor[3].read());
	};
	void sensorTestwithAlgo(){
		Serial.print(sensor[0].isBlack());
		Serial.print(" / ");
		Serial.print(sensor[1].isBlack());
		Serial.print(" / ");
		Serial.print(sensor[2].isBlack());
		Serial.print(" / ");
		Serial.println(sensor[3].isBlack());

	};
	void Brake(){
		if(carDirection == Motor::Stop) return;
		setDirection(carDirection == Motor::Forward? Motor::Backward : Motor::Forward);
		setStatusLight(Motor::Stop);
		delay(150);
		setDirection(Motor::Stop);
		delay(130);
	};
	void turnRight(){
		rotate(Motor::Right);
		delay(180);
		setDirection(Motor::Stop);
		delay(250);
	};
	void turnLeft(){
		rotate(Motor::Left);
		delay(180);
		setDirection(Motor::Stop);
		delay(250);
	};
	void turnRightwithSensor(){
		rotate(Motor::Right);
		delay(150);
		while(!sensor[3].isBlack()) rotate(Motor::Right);
		rotate(Motor::Left);
		delay(100);
		setDirection(Motor::Stop);
		delay(80);
	}
	void turnLeftwithSensor(){
		rotate(Motor::Left);
		delay(150);
		while(!sensor[0].isBlack()) rotate(Motor::Left);
		rotate(Motor::Right);
		delay(100);
		setDirection(Motor::Stop);
		delay(80);
	}
	void turnBack(){
		rotate(Motor::Left);
		delay(390);
		setDirection(Motor::Stop);
		delay(250);
	};
	void setStatusLight(Motor::Direction Dir){
		digitalWrite(StatusLightRed, Dir == Motor::Stop? LOW : HIGH);
		digitalWrite(StatusLightYellow,Dir == Motor::Backward? LOW : HIGH);
		digitalWrite(StatusLightGreen, Dir == Motor::Forward? LOW : HIGH);
	};
	void setLightDetected(){
		digitalWrite(StatusLightYellow,LOW);
		digitalWrite(StatusLightGreen,LOW);
		digitalWrite(StatusLightRed,HIGH);
	}

	private:
	void lineKeeping();

};

void Car::lineKeeping(){
	if(sensor[1].isBlack()){
		Left.setSpeed(120);
		Right.setSpeed(220);
	}
	else if(sensor[2].isBlack()){
		Left.setSpeed(220);
		Right.setSpeed(120);
	}
	else setSpeed(carSpeed);
}

void Car::rotate(Motor::Direction dir){
	if(dir == Motor::Right){
		digitalWrite(Left.pin.A, HIGH);
		digitalWrite(Right.pin.A, LOW);
		digitalWrite(Left.pin.B, LOW);
		digitalWrite(Right.pin.B, HIGH);
	}
	else if(dir == Motor::Left){
		digitalWrite(Left.pin.A, LOW);
		digitalWrite(Right.pin.A, HIGH);
		digitalWrite(Left.pin.B, HIGH);
		digitalWrite(Right.pin.B, LOW);
	}
}

void Car::setDirection(Motor::Direction direction){
	setStatusLight(direction);
	if(direction == Motor::Forward){
		digitalWrite(Left.pin.A, HIGH);
		digitalWrite(Right.pin.A, HIGH);
		digitalWrite(Left.pin.B, LOW);
		digitalWrite(Right.pin.B, LOW);
	}
	else if(direction == Motor::Backward){
		digitalWrite(Left.pin.A, LOW);
		digitalWrite(Right.pin.A, LOW);
		digitalWrite(Left.pin.B, HIGH);
		digitalWrite(Right.pin.B, HIGH);
	}
	else if(direction == Motor::Stop){
		digitalWrite(Left.pin.A, LOW);
		digitalWrite(Right.pin.A, LOW);
		digitalWrite(Left.pin.B, LOW);
		digitalWrite(Right.pin.B, LOW);
	}
	else return;
	carDirection = direction;
}

void Car::ActivateLineFollowing(){
	const int SPEED = 200;
	delay(500);
	setSpeed(SPEED);
	setDirection(Motor::Forward);

	//Go to 1st Intersection
	while(!sensor[3].isBlack()){
		sensorTest();
		lineKeeping();
	}
	setLightDetected();

	//1st Intersection
	delay(125);
	Brake();
	turnRightwithSensor();
	setDirection(Motor::Forward);

	//Red Checkpoint
	while(!(sensor[0].isBlack() and sensor[3].isBlack())) lineKeeping();
	setLightDetected();
	delay(200);
	Brake();

	//Back form Red Chkp
	setSpeed(100);
	setDirection(Motor::Backward);
	delay(750);
	setSpeed(SPEED);
	Brake();

	//Go to Blue Chkp
	turnRightwithSensor();
	for(int i = 0;i < 3;++i){
		setDirection(Motor::Forward);
		while(!sensor[0].isBlack()) lineKeeping();
		setLightDetected();
		delay(125);
		Brake();
		if(i < 2) turnLeftwithSensor();
	}

	turnRightwithSensor();

	//Get into Blue
	setSpeed(100);
	setDirection(Motor::Backward);
	delay(750);
	setSpeed(SPEED);
	Brake();

	//Back from Blue Chkp - Forward
	setDirection(Motor::Forward);
	const unsigned long long time = millis();
	while(millis() - time <= 500) lineKeeping();
	while(!(sensor[3].isBlack())) lineKeeping();
	setLightDetected();
	delay(125);
	Brake();

	//Final Intersection
	turnRightwithSensor();
	setDirection(Motor::Forward);

	//Go to Finish Point
	while(!(sensor[0].isBlack() && sensor[1].isBlack() && sensor[2].isBlack() && sensor[3].isBlack())) lineKeeping();
	setLightDetected();
	delay(180);
	Brake();
}



 int sensorList[] = {A0,A1,A2,A3};

Car car({11,10,9},{3,5,6},sensorList,-2,150);


void setup() {
	car.pinInit();
	Serial.begin(9600);
}

void loop() {
	car.ActivateLineFollowing();
}