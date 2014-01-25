
//
// The application is used to sense the maximum g-force of a short-lived (<500ms) event.
// This program drives the Bosch BMA180 digital 3-axis acceleration sensor using I2C via the Arduino Wire interface. 
//
// Inspired by code posted by wm.paul.bennett posted to bild.forum
//     http://forum.bildr.org/viewtopic.php?f=25&t=80
//
// Refers to Bosch documentation.
//     BMA180 data sheet (2010/12/09) for details on the chip and the I2C interface.

#include <Wire.h>

//
// BMA chip address
//
// The 7 bit address of the BMA chip as specified in the data sheet is (0x40).
// This is required to be shifted over a bit when broadcast. The Wire library hides
// this from you. Use the raw value specified in the datasheet.
//
#define I2C_ADDR_BMA      0x40
#define I2C_ADDR_SLAVE    (I2C_ADDR_BMA)
#define NAXIS             3             

// BMA register addresses
// (see also p.21 of the data sheet)
//
#define R_CHIP_ID        0x00
#define R_VERSION        0x01
#define R_ACC_X_LSB      0x02
#define R_ACC_X_MSB      0x03
#define R_ACC_Y_LSB      0x04
#define R_ACC_Y_MSB      0x05
#define R_ACC_Z_LSB      0x06
#define R_ACC_Z_MSB      0x07
#define R_CTRL_REG0      0x0D
#define R_BW_TCS         0x20
#define R_CUST_DATA1     0x2C
#define R_OFFSET_LSB1    0x35


// String constants (NOTE: program text is loaded into more abundant Flash memory)
#define S_COMMA            ","

void setup() {
  Serial.begin(19200);
  Serial.println("setup");
  digitalWrite(A4, HIGH);        // TODO: enable pullup; is this required?
  digitalWrite(A5, HIGH);        // TODO: enable pullup; is this required?
  delay(1000);                   // The BMA needs a half second to start from power up
  Wire.begin();                  // Initialize I2C, i2c automagically enables pull-ups
  
  initBMA();                     // set up the BMA device
}

void loop() {

  // Wait until BMA180 is quiescent (no acceleration)
  int nsamples = 50;
  int tolerance = 200;
  int now[NAXIS];
  int atRest[NAXIS];
  
  // determine the atRest values for (x,y,z) acceleration
  do {
    Serial.println("waiting to be atRest");
    getMeanAccel(nsamples, atRest);
    delay(500);
    getMeanAccel(nsamples, now);
  } while (different(atRest,now,tolerance));
  displayOK();

  // Wait for the start of an acceleration event; each loop takes ~3ms
  Serial.println("atRest, waiting for accel ...");
  do {
    getAccelData(now); 
  } while( (abs(now[0]-atRest[0])<tolerance) && (abs(now[1]-atRest[1])<tolerance) && (abs(now[2]-atRest[2])<tolerance));
  
  collectAccelData();
  displayAccel();
  delay(1000);

}

void initBMA() {
  // turn on ee_w to write to volatile image register memory (p.26)
  byte val = getRegister(R_CTRL_REG0);
  setRegister(R_CTRL_REG0, val | 0b00010000); 
  
  // set low-pass filter to 10hz to get better signal/noise (p.27)
  val = getRegister(R_BW_TCS);
  setRegister(R_BW_TCS, val & 0x0F);

//  
// 3-bit value denoting acceleration range and ADC resolution
// 000 - 1.0g = 0.13 mg/LSB
// 001 - 1.5g = 0.19 mg/LSB
// 010 - 2.0g = 0.25 mg/LSB
// 011 - 3.0g = 0.38 mg/LSB
// 100 - 4.0g = 0.50 mg/LSB
// 101-  8.0g = 0.99 mg/LSB
// 111- 16.0g = 1.98 mg/LSB
  val = getRegister(R_OFFSET_LSB1);
  val &= 0b11110001;
  val |= 0b00001000;  // set range = 4g
  setRegister(R_OFFSET_LSB1, val);

}

///////////////////////////////////////////////////////
//
// BMA180 sensor state processing
//
void getMeanAccel(int nsamples, int* meanvals) {
  double sumx = 0;
  double sumy = 0;
  double sumz = 0;
  int vals[NAXIS] = {0,0,0};
  
  for (int i=0; i<nsamples; i++) {
    getAccelData(vals);
    sumx += (double) vals[0];
    sumy += (double) vals[1];
    sumz += (double) vals[2];
  }
  meanvals[0]=(int) (sumx/(double) nsamples);
  meanvals[1]=(int) (sumy/(double) nsamples);
  meanvals[2]=(int) (sumz/(double) nsamples);
}


boolean different(int* vals_1, int* vals_2, int tolerance) {
  boolean same = true;
  int i=0;
  do {
    same &= ( abs(vals_1[i] - vals_2[i]) <= tolerance);
  } while (same && (++i < NAXIS));
  return (!same);
}

void displayOK() {
}

void displayAccel() {
}

void collectAccelData() {
  int n=200;
  int xa[n];
  int ya[n];
  int za[n];
  int vals[3];

  int start = millis();
  for (int i=0; i<n; i++) {
    getAccelData(vals);
    xa[i] =vals[0];
    ya[i] =vals[1];
    za[i] =vals[2];
  }
  int end =millis();

  double max_range = (double) 0b0001111111111111; // 8192 - max value for 14-bit signed number
  double max_g = 4;;
  printStats("x",n,xa);
  printStats("y",n,ya);
  printStats("z",n,za);

  // compute max acceleration during data collection period
  double mmax = 0;
  double magnitude, dx, dy, dz;
  for(int i=0; i<n; i++) {
    dx = (double) xa[i] * (double) xa[i]; 
    dy = (double) ya[i] * (double) ya[i]; 
    dz = (double) za[i] * (double) za[i]; 
    magnitude = sqrt(dx+dy+dz);
    int vals[] ={xa[i],ya[i],za[i],(int) magnitude};
    //printAccelData(vals);
    if (magnitude > mmax) {
        mmax = magnitude;
    }
  } 
  
  Serial.print("total time:");
  Serial.println(end-start);
  Serial.print("MAX magnitude: ");
  Serial.print(mmax);
  Serial.print("  MAX Accel: ");
  Serial.print( (mmax/max_range) * (max_g));
  Serial.println(" g");
}

///////////////////////////////////////////////////////
//
// BMA180 get/set primitives
//



int getRegister(byte registerAddr) {
  Wire.beginTransmission(I2C_ADDR_SLAVE);
  Wire.write(registerAddr);
  Wire.endTransmission();
  Wire.requestFrom(I2C_ADDR_SLAVE,1); // request numbytes bytes
  
  int n=Wire.available();
  if (n==0) {
    Serial.println("getRegister: error: 0 bytes available");
  }
  //printRegisterOperation(true,registerAddr,val);   
  return Wire.read();
}

void setRegister(byte registerAddr, byte val) {
  byte xmit[] = { registerAddr, val  };
  Wire.beginTransmission(I2C_ADDR_SLAVE);
  Wire.write(xmit,2);
  Wire.endTransmission();
  //printRegisterOperation(false,registerAddr,val);   
}

///////////////////////////////////////////////////////
//
// BMA180 get/set functions
//

//
// Fetch 6 bytes from sequential registers -- 2bytes for x,y,z values.
// Only 14 MSB of these values are significant.
//
// NOTE: With the BMA 180 and Arduino Uno running this code, I am getting timings of <2ms to 'getAccelData'
//
void getAccelData(int* values) {
  Wire.beginTransmission(I2C_ADDR_SLAVE);
  Wire.write(R_ACC_X_LSB);
  Wire.endTransmission();
  Wire.requestFrom(I2C_ADDR_SLAVE,6);
  values[0] = (Wire.read() | (Wire.read() << 8)) >> 2;
  values[1] = (Wire.read() | (Wire.read() << 8)) >> 2;
  values[2] = (Wire.read() | (Wire.read() << 8)) >> 2;
}

inline int getXAccel() {
  byte msb = getRegister(R_ACC_X_MSB); 
  byte lsb = getRegister(R_ACC_X_LSB);
  return (((msb << 8) | lsb) >>2); 
}

inline int getYAccel() {
  byte msb = getRegister(R_ACC_Y_MSB);
  byte lsb = getRegister(R_ACC_Y_LSB);
  return (((msb << 8) | lsb) >>2); 
}

inline int getZAccel() {
  byte msb = getRegister(R_ACC_Z_MSB);
  byte lsb = getRegister(R_ACC_Z_LSB);
  return (((msb << 8) | lsb) >>2); 
}

///////////////////////////////////////////////////////
//
// Arithmetic functions
//

int getMean(int n, int num[]) {
  double sum = 0;
  for (int i=0; i<n; i++) {
    sum += (double)num[i];
  }
  return  (int) (sum/(double)n);
}

int getMin(int n, int num[]) {
  int min = num[0];
  for (int i=0; i<n; i++) {
    min = (num[i] < min)  ? num[i] : min;
  }
  return min;
}

int getMax(int n, int num[]) {
  int max = num[0];
  for (int i=0; i<n; i++) {
    max = (num[i] > max)  ? num[i] : max;
  }
  return max;
}

// stddev calculated in one pass 
double getStdDev(int n, int num[]) {
  double sum=0;
  double sumsq=0;
  double dnum=0;
  for (int i=0; i<n; i++) {
    dnum = (double) num[i];
    sum += dnum;
    sumsq += dnum*dnum;
  }
  return sqrt( (sumsq- (sum*sum/(double)n))/ ((double)(n-1)) );
}


///////////////////////////////////////////////////////
//
// Debugging functions
//

void printStats(const char* name, int n, int num[]) {
  int min = getMin(n,num);
  int max = getMax(n,num);
  int mean = getMean(n,num);
  double stddev = getStdDev(n,num);
  Serial.print(name);
  Serial.print("=(");
  Serial.print(min); 
  Serial.print(S_COMMA);
  Serial.print(mean); 
  Serial.print(S_COMMA);
  Serial.print(max);  
  Serial.print(S_COMMA);
  Serial.print(stddev); 
  Serial.print(") ");
}

void printRegisterOperation(boolean get, byte registerAddr, byte value) {
  const char* str = get ? "GOT: " : "SET:";
  Serial.print(str);
  Serial.print(registerAddr, HEX);
  Serial.print(":");
  Serial.print(value,BIN);
  Serial.println(":");
} 

void printAccelData (int* vals) {
    Serial.print("accel (x=");
    Serial.print(vals[0]); 
    Serial.print(", y=");
    Serial.print(vals[1]); 
    Serial.print(", z=");
    Serial.print(vals[2]);
    Serial.print(" mag=");
    Serial.print(vals[3]); 
    Serial.println(") ");
}

///////////////////////////////////////////////////////
//
// Test functions
//

void test_readwrite() {
  byte val = getRegister(R_CTRL_REG0);
  //setRegister(R_CTRL_REG0, val & 0b11101111); // turn off ee_w to write to volatile image register memory
  setRegister(R_CTRL_REG0, val | 0b00010000); // turn on ee_w to write to volatile image register memory
  val = getRegister(R_CTRL_REG0);
  val = getRegister(R_CUST_DATA1);
  setRegister(R_CUST_DATA1,4);
  val = getRegister(R_CUST_DATA1);
  setRegister(R_CUST_DATA1,22);
  val = getRegister(R_CUST_DATA1);
  delay(10000);
}

void test_dataCollection() {
  int n=50;
  int xa[n];
  int ya[n];
  int za[n];

  Serial.println("loop");
  int start = millis();
  for (int i=0; i<n; i++) {
    xa[i] =getXAccel();
    ya[i] = getYAccel();
    za[i] = getZAccel();
  }
  int end =millis();
  Serial.print("total time:");
  Serial.println(end-start);
  printStats("x",n,xa);
  printStats("y",n,ya);
  printStats("z",n,za);
  delay(5000);
  Serial.println("GO!");
  delay(500);
}
