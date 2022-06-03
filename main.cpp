
#include "bcm2835.h" // https://www.airspayce.com/mikem/bcm2835/
#include <chrono>
#include <signal.h>
#include <stdio.h>
#include <thread>
#include <unistd.h>

void usleep(int us)
{
	std::this_thread::sleep_for(std::chrono::microseconds(us));
}

void msleep(int ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

#define PIN_RDYBSY RPI_V2_GPIO_P1_03
#define PIN_OE RPI_V2_GPIO_P1_05
#define PIN_WR RPI_V2_GPIO_P1_07
#define PIN_BS1 RPI_V2_GPIO_P1_11
#define PIN_XA0 RPI_V2_GPIO_P1_13
#define PIN_XA1 RPI_V2_GPIO_P1_15
#define PIN_PAGEL RPI_V2_GPIO_P1_19
#define PIN_XTAL1 RPI_V2_GPIO_P1_21
#define PIN_BS2 RPI_V2_GPIO_P1_23

#define PIN_DATA0 RPI_V2_GPIO_P1_08
#define PIN_DATA1 RPI_V2_GPIO_P1_10
#define PIN_DATA2 RPI_V2_GPIO_P1_12
#define PIN_DATA3 RPI_V2_GPIO_P1_16
#define PIN_DATA4 RPI_V2_GPIO_P1_18
#define PIN_DATA5 RPI_V2_GPIO_P1_22
#define PIN_DATA6 RPI_V2_GPIO_P1_24
#define PIN_DATA7 RPI_V2_GPIO_P1_26

enum PinMode {
	Input,
	InputPullUp,
	InputPullDown,
	Output
};

template <int N> class GPIO {
public:
private:
	PinMode mode_ = Input;
public:
	GPIO()
	{
		set_mode(Input);
	}
	void set_mode(PinMode m)
	{
		mode_ = m;
		switch (mode_) {
		case PinMode::Input:
			bcm2835_gpio_set_pud(N, BCM2835_GPIO_PUD_OFF);
			bcm2835_gpio_fsel(N, BCM2835_GPIO_FSEL_INPT);
			break;
		case PinMode::InputPullUp:
			bcm2835_gpio_set_pud(N, BCM2835_GPIO_PUD_UP);
			bcm2835_gpio_fsel(N, BCM2835_GPIO_FSEL_INPT);
			break;
		case PinMode::InputPullDown:
			bcm2835_gpio_set_pud(N, BCM2835_GPIO_PUD_DOWN);
			bcm2835_gpio_fsel(N, BCM2835_GPIO_FSEL_INPT);
			break;
		case PinMode::Output:
			bcm2835_gpio_fsel(N, BCM2835_GPIO_FSEL_OUTP);
			break;
		}
	}
	bool read()
	{
		return bcm2835_gpio_lev(N);
	}
	void write(bool b)
	{
		if (b) {
			bcm2835_gpio_set(N);
		} else {
			bcm2835_gpio_clr(N);
		}
	}
};


class AVR {
private:
	GPIO<PIN_RDYBSY> pin_rdybsy_;
	GPIO<PIN_OE> pin_oe_;
	GPIO<PIN_WR> pin_wr_;
	GPIO<PIN_BS1> pin_bs1_;
	GPIO<PIN_BS2> pin_bs2_;
	GPIO<PIN_XA0> pin_xa0_;
	GPIO<PIN_XA1> pin_xa1_;
	GPIO<PIN_PAGEL> pin_pagel_;
	GPIO<PIN_XTAL1> pin_xtal1_;
	GPIO<PIN_DATA0> pin_data0_;
	GPIO<PIN_DATA1> pin_data1_;
	GPIO<PIN_DATA2> pin_data2_;
	GPIO<PIN_DATA3> pin_data3_;
	GPIO<PIN_DATA4> pin_data4_;
	GPIO<PIN_DATA5> pin_data5_;
	GPIO<PIN_DATA6> pin_data6_;
	GPIO<PIN_DATA7> pin_data7_;
	void set_programming_mode(bool f)
	{
		if (f) {
			write_oe(1);
			write_wr(1);
			write_xa0(0);
			write_xa1(0);
			write_bs1(0);
			write_bs2(0);
			write_xtal1(0);
			write_pagel(0);
			set_data_pin_mode(PinMode::Output);
			usleep(1);
		} else {
			pin_rdybsy_.set_mode(PinMode::Input);
			pin_oe_.set_mode(PinMode::Input);
			pin_wr_.set_mode(PinMode::Input);
			pin_bs1_.set_mode(PinMode::Input);
			pin_bs2_.set_mode(PinMode::Input);
			pin_xa0_.set_mode(PinMode::Input);
			pin_xa1_.set_mode(PinMode::Input);
			pin_pagel_.set_mode(PinMode::Input);
			pin_xtal1_.set_mode(PinMode::Input);
			set_data_pin_mode(PinMode::Input);
		}
	}
	void write_data_pins(bool iscommand, uint8_t value)
	{
		write_data_pins(value);
		set_data_pin_mode(PinMode::Output);
		write_xa1(iscommand);
		pulse_xtal1();
		write_xa1(0);
		set_data_pin_mode(PinMode::Input);
	}
	bool read_rdybsy()
	{
		return pin_rdybsy_.read();
	}
	void write_oe(bool b)
	{
		pin_oe_.write(b);
	}
	void write_wr(bool b)
	{
		pin_wr_.write(b);
	}
	void write_bs1(bool b)
	{
		pin_bs1_.write(b);
	}
	void write_bs2(bool b)
	{
		pin_bs2_.write(b);
	}
	void write_xa0(bool b)
	{
		pin_xa0_.write(b);
	}
	void write_xa1(bool b)
	{
		pin_xa1_.write(b);
	}
	void write_pagel(bool b)
	{
		pin_pagel_.write(b);
	}
	void write_xtal1(bool b)
	{
		pin_xtal1_.write(b);
	}
	void set_data_pin_mode(PinMode mode)
	{
		pin_data0_.set_mode(mode);
		pin_data1_.set_mode(mode);
		pin_data2_.set_mode(mode);
		pin_data3_.set_mode(mode);
		pin_data4_.set_mode(mode);
		pin_data5_.set_mode(mode);
		pin_data6_.set_mode(mode);
		pin_data7_.set_mode(mode);
	}
	uint8_t read_data_pins()
	{
		return pin_data0_.read()
				| (pin_data1_.read() << 1)
				| (pin_data2_.read() << 2)
				| (pin_data3_.read() << 3)
				| (pin_data4_.read() << 4)
				| (pin_data5_.read() << 5)
				| (pin_data6_.read() << 6)
				| (pin_data7_.read() << 7);
	}
	void write_data_pins(uint8_t c)
	{
		pin_data0_.write(c & 1);
		pin_data1_.write((c >> 1) & 1);
		pin_data2_.write((c >> 2) & 1);
		pin_data3_.write((c >> 3) & 1);
		pin_data4_.write((c >> 4) & 1);
		pin_data5_.write((c >> 5) & 1);
		pin_data6_.write((c >> 6) & 1);
		pin_data7_.write((c >> 7) & 1);
	}
	void pulse_xtal1()
	{
		usleep(1);
		write_xtal1(1);
		usleep(1);
		write_xtal1(0);
		usleep(1);
	}
public:
	AVR()
	{
		pin_rdybsy_.set_mode(PinMode::Input);
		pin_oe_.set_mode(PinMode::Output);
		pin_wr_.set_mode(PinMode::Output);
		pin_bs1_.set_mode(PinMode::Output);
		pin_bs2_.set_mode(PinMode::Output);
		pin_xa0_.set_mode(PinMode::Output);
		pin_xa1_.set_mode(PinMode::Output);
		pin_pagel_.set_mode(PinMode::Output);
		pin_xtal1_.set_mode(PinMode::Output);
		set_data_pin_mode(PinMode::Input);
	}
	~AVR()
	{
		set_programming_mode(false);
	}
	void set_command(uint8_t c)
	{
		write_data_pins(true, c);
	}
	void set_low_address(uint8_t c)
	{
		write_data_pins(false, c);
	}
	uint8_t read_data()
	{
		set_data_pin_mode(PinMode::Input);
		write_bs1(0);
		write_oe(0);
		usleep(1);
		int c = read_data_pins();
		write_oe(1);
		write_bs1(0);
		return c;
	}
	uint8_t read_signature(int addr)
	{
		set_command(0x08);
		set_low_address(addr);
		return read_data();
	}
};

AVR *avr;

void sigint(int i)
{
	delete avr;
	bcm2835_close();
	exit(0);
}

int main()
{
	signal(SIGINT, sigint);
	bcm2835_init();
	avr = new AVR();

	avr->set_command(0x08);
	avr->set_low_address(0x00);
	int c = avr->read_data();

	printf("%02x %02x %02x\n"
		   , avr->read_signature(0)
		   , avr->read_signature(1)
		   , avr->read_signature(2)
		   );

	delete avr;
	return 0;
}

