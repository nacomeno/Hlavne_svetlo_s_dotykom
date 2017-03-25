/*
*******=================== hlavne svetlo s dotykom 1.c ==================********
___________________________________________________________________________________

*			 procesor ATMEGA 8 - 16PU
*			 frekvencia krystalu 8MHz
*			 preddelicka (prescaler) 8
*				perioda inkrementacie TNCT0 = 1us
*				rucne nastavenie TNCT0 aby pretecenie nastalo vzdy po 250us ---------- TNCT0=6
*			 Port B
*				PB6,7 - pripojenie krystalu
*				ostatne vyuzite len na ISP programovanie	
*			 Port C 
*				PC0, PC1, PC2 - piny pre vyhodnotenie dotyku na "rotacnom enkoderi"
*				PC3 - dotykova plocha pre zap./vyp.
*				PC4 - debuggovacia LED (anoda na pin, katoda na zem NEZAPINAT PULL UP REZISTOR!!!!) ------------ casom mozno vyhodit
*				PC5 - vystup na spinaci prvok (NEZAPINAT PULL UP REZISTOR !!!!)
*			 Port D
*				PD0,1 - bezdrotova komunikacia ????????????????????
*				PD2 - ext. prerusenie INT0 detektor priechodu nulou
*				ostatne nevyuzite
*/ 

#define FCPU 8000000
#include <avr/io.h>
#include <avr/interrupt.h>

uint8_t j = 0;						//urcenie  tlacida
uint8_t temp = 0;					//docasne ulozenie hodnoty dotyku
volatile uint8_t temp1 = 0;			//vystupna hodnota z cakacej funkcie bude porovnavat cas kolko nebolo stlacene ziadne tlacidlo
volatile uint8_t count = 0;			//pocita pocet priechodov sietoveho napatia nulou a podla toho odpocitava cas 0,5s
volatile uint8_t znacka = 0;		//predstavuje premennu z casovaca pri kazdom preteceni t.j. 250us sa navysi znacka <= 39 (9,75 ms ... dlzka trvania jednej polperiody je 10ms)
volatile uint8_t i = 0;				//casova hodnota doby zopnuteho svetla t.j. cas = i*250us
volatile uint8_t tlac = 0;			//priznak dotyku na tlacitku 


ISR(TIMER0_OVF_vect)				// prerusenie pretecenia casovaca
{	
	TCNT0 = 6;						// nastavi pociatocnu hodnotu casovaca TNCT0
	if (znacka < 39)
	znacka++;						// pri kazdom preteceni inkrementuje znacku (znacka = pocet preteceni za 1 interval)
	if (i > 0)
	{if (znacka == i)				//ak sa rovna hodnota nastavena enkoderom so znackou vypne vystup ---- znacka *250us = cas v jednej polvlne za ktory ma vypnut
		PORTC &=~ ((1 << PC4) | (1 << PC5));			//vypne vystupy
	}
}

ISR(INT0_vect)						//prerusenie priechodom sietoveho napatia nulou
{
	TCNT0 = 6;						// ak preslo sietove napatie nulou nastavi pociatocnu hodnotu TNCT0 (synchronizuje TNCT0 s priechodmi sietoveho napatia nulou) 
	znacka = 0;
	
	if (i > 0)						//zisti enkoderom nastavenu hodnotu oneskorenia
		{PORTC |= ((1 << PC4) | (1 << PC5));			//ak ma svetlo svietit t.j i>0 zapne vystup
		} 
		else
			{PORTC &=~ ((1 << PC4) | (1 << PC5));		//ak ma byt zhasnute t.j. i==0 vypne vystup
			}
	if (count < 3)					//cakacia funkcia 10ms (kazdych 10ms skontroluje dalsie tlacidlo)
		count++;
		else
		{count = 1;					//kazdych 30ms skontroluje priznak pre tlacitko 
			if (tlac == 1)			//ak je priznak nastaveny na vypnutie kazdych 30ms uberie 250us z nastaveneho casu			
			{i--;
				if (i == 0)
				tlac = 0;
			}
			else if (tlac == 2)		//ak je priznak nastaveny na zapnutie tak kazdych 30ms prirata 250us t.j. do plneho svitu je to cas priblizne 1,17s
			{i++;
				if (i == 39)
				tlac = 0;
			}
		}
		
	if (temp1 < 50)					//podla tohto vyhodnoti ci bolo tlacidlo stlacene do 500ms od stlacenia posledneho tlacidla
		temp1 ++;	
}


void smer()							//funkcia na kontrolu tlacidiel
{
	if (temp != j)					//ak sa nerovnaju cize bolo stlacene nejake ine tlacidlo
	{
		if (j == 1)					//ak bola stlacena jednotka
		{
			if (temp == 2)			//otestuje ci predtym nebola stlacena dvojka a ak hej ak svetlo svieti uberie z konstanty
			{if (i > 0 )
				i--;
			}
			else
			{if (temp == 3)			//otestuje ci predtym nebola stlacena trojka a ak hej ak svetlo nie je na maxime prida konstante
				{if (i < 39)
					i++;
				}
			}
			temp = 0;				//ak sa rovnaju vynuluje temp cize ako keby tlacidlo bolo stale stlacene 
		}
		if (j == 2)					//ak bola stlacena dvojka
		{
			if (temp == 3)			//otestuje ci predtym nebola stlacena trojka a ak hej ak svetlo svieti uberie z konstanty
			{if (i > 0)
				i--;
			}
			else if (temp == 1)		//otestuje ci predtym nebola stlacena jednotka a ak hej ak svetlo nie je na maxime prida konstante
			{if (i < 39)
				i++;
			}
			temp = 0;				//ak sa rovnaju vynuluje temp cize ako keby tlacidlo bolo stale stlacene 
		}
		if (j == 3)					//ak bola stlacena trojka
		{
			if (temp == 1)			//otestuje ci predtym nebola stlacena jednotka a ak hej ak svetlo svieti uberie z konstanty
			{if (i > 0)
				i--;
			}
			else if (temp == 2)		//otestuje ci predtym nebola stlacena dvojka a ak hej ak svetlo nie je na maxime prida konstante
			{if (i < 39)
				i++;
			}
			temp = 0;				//ak sa rovnaju vynuluje temp cize ako keby tlacidlo bolo stale stlacene
		}
	}
}



void dlha_doba()					//ak nie je ziadne tlacidlo stlacene dlhsie ako 0,5s (casom mozna zmena na iny cas)
{
	temp = 0;
	j = 0;
	if (PINC & ((1 << PC0 )) || (PINC & (1 << PC1 )) ||  (PINC & (1 << PC2 )))				//skontroluj naraz vsetky piny ci nie je nieco stlacene ak je vynuluj cakaciu konstantu
	{
		temp1 = 0;
		count = 1;
		
	}
}

void kratka_doba()					//dotyk do 0,5s
{
	switch (count){
		
		case 1:
		{if (PINC & (1 << PC0 ))
			{temp = j;
				j = 1;
			temp1 = 0;}
		}
		break;
		
		case 2:
		{if (PINC & (1 << PC1 ))
			{temp = j;
				j = 2;
			temp1 = 0;}
		}
		break;
		
		case 3:
		{if (PINC & (1 << PC2 ))
			{temp = j;
				j = 3;
			temp1 = 0;}
		}
		break;
	}
	smer();
}

void tlacitko()						//vyhodnocovanie dotyku na tlacitku 
{
	if (PINC & (1 << PC3 ))			//ak je log. 1 na PC3 t.j. dotyk na tlacitku
	{if (i > 0)
		tlac = 1;					//ak svetlo svieti tlac = 1 je priznak pre pomale vypnutie
		else tlac = 2;				//ak svetlo nesvieti tlac = 2 je priznak pre pomale zapnutie do maxima 
	}
}		


int main()
{
	DDRC &= ~ ((1 << PC0) | (1 << PC1) | (1 << PC2) | (1 << PC3) );			//PC0,1,2,3 ako vstupne
	DDRC |= ((1 << PC4) | (1 << PC5));										//PC4,5 ak vystupne
	DDRD &= ~ ((1 << PD0) | (1 << PD2)) ;									//PD0,2 ako vstupne
	
	
	MCUCR |= (1 << ISC01);			// dobežná hrana INT0
	GICR |=  (1 << INT0);			//povoli prerusenia INT0	
	TCCR0 |= (1 << CS01);			//predelicka Clko/8
	TIMSK |= (1 << TOIE0);			//prerusenie pri preteceni Timer0
	sei();							//povolenie preruseni
	TCNT0 = 6;
	
    while(1)
    {
		
		if (temp1 == 50)			//ak ziadne tlacidlo nebolo stlacene dlhsie ako 500ms
		
		dlha_doba();
		
		if (temp1 < 50)				//ak bolo nieco stlacene pred uplynutim 500ms
		
		kratka_doba();
		
		tlacitko();					//overenie dotyku na tlacitku
    }
}