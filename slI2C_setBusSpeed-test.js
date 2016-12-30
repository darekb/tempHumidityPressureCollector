var F_CPU = 16000000;
var I2CBUSCLOCK = 100000
var speed = I2CBUSCLOCK /100;
var prescaler = 0;
speed = (F_CPU / speed / 100 - 16) / 2;
while (speed > 255) {
	prescaler++;
	speed = speed / 4;
};
console.log('speed: ', speed);
console.log('prescaler: ', prescaler);
//TWSR = (0 & ((1 << 1) | (1 << 0))) | 0;
//TWBR = speed;

console.log('speed2: ', ((F_CPU / I2CBUSCLOCK) - 16) / 2);
