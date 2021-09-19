#include <stdio.h>							 
#include <REG8253.H> //including sfr registers for ports of the controller
#include <lcd.h>
#define tempoDeDesligamento(min) ((min*60)/0.065535) 

//VARIAVEIS PARA A COMUNICACAO COM O CONVERSOR ANALOGICO DIGITAL
sbit cs=P3^5;
sbit miso=P1^6;
sbit mosi=P1^5;
sbit clk=P1^7;

//LCD Module Connections
sbit RS = P2^0;                                                                   
sbit EN = P2^1;
sbit D4 =  P2^2; 
sbit D5 =  P2^3; 
sbit D6 =  P2^4; 
sbit D7 =  P2^5; 
//End LCD Module Connections

//PROTOTIPO DAS FUNCOES QUE SERAO UTILIZADAS
void Timer0();
void TrataIntExt0(void);
void Externa1(void);
void Timer1();
void Lcd4_init();
float readadc(bit d1,bit d0);
float conversaoTensaoEmTemperatura(float temperaturaEmVolts);


//FUNCAO DE ATRASO NA EXECUCAO
void Delay(int a)
{
	char j;
	int i;
	for(i=0;i<a;i++)
	{
		for(j=0;j<100;j++)
		{
		}
	}
}

//VARIAVEIS GLOBAIS
unsigned int temporizadorGeral=0;
unsigned int defTemp=2;
float temperatura = 0;
char temp1[6];
char setpoint[4];
char flagBoucing = 0;
unsigned int counter=200; //temperatura minima para funcionamento
char tempoBeep=0; //utilizado para temporizar o beep por 1s sem delay
bit opcao=1;
char tempDesl[4];
bit flagTempDesl=1;//Inicio com 1 para entrar so uma vez nas definicoes correlatas a flag

void main()
{ 
	EA=1; // Habilita interrupcoes em geral
	EX0=1; // Habilita interrupcao Externa 0 P3.2
	IT0=1; // Conf , Sensibilida e Transicao. COMUTACAO de DESCIDA
	EX1=1; // Habilita interrupcao Externa 1 P3.3
	IT1=1; // Conf , Sensibilida e Transicao. COMUTACAO de DESCIDA

	ET0=1; // ENABLE TIMER0
	ET1=1; // ENABLE TIMER1
	TR0=1;	// ENABLE OVERFLOW INTERRUPTS ON TIMER0
	TR1=1; // ENABLE OVERFLOW INTERRUPTS ON TIMER1
	TMOD=0x11; //00010001b Configura Timer0 16bits e Timer1 16bits
	TH0=0xFC;
	TL0=0x18;

	TH1=0x00;
	TL1=0x00;
	P3_4=1;//Pino de beep. Beep desativado
	P3_1=1;//Ferro inicia desligado
	sprintf(tempDesl,"%03d",defTemp);
	Lcd4_init();
    Lcd4_Clear();
    Lcd4_Set_Cursor(1,1);
   	Lcd4_Write_String("EST. DE SOLDA");
    Delay(700);
    Lcd4_Clear();
    Lcd4_Set_Cursor(1,0);
    Lcd4_Write_String("Tempo Desl. SET");
    Lcd4_Set_Cursor(2,0);
    Lcd4_Write_String(tempDesl);
    Delay(700);
    Lcd4_Clear();
    Lcd4_Set_Cursor(1,0);
    Lcd4_Write_String("P/ alter. press.");
    Lcd4_Set_Cursor(2,0);
    Lcd4_Write_String("o bot. d encoder");
    Delay(1000);
    Lcd4_Clear();
   	
    	while(1)
    	{
    		if(temporizadorGeral<tempoDeDesligamento(defTemp))//contador incrementado pelo timer1
    			
    		{
    			if(opcao)
    			{
		    		temperatura = readadc(0,0);
		    		//funcao abaixo para transformar a tensao(em volts) em temperatura
		    		temperatura=conversaoTensaoEmTemperatura(temperatura);
		    		sprintf(temp1,"%06.2f",temperatura);//preenchimento de zeros para evitar uma contagem errada do sprintf
		    	    Lcd4_Set_Cursor(1,0);
		    		Lcd4_Write_String("TEMP: ");
		    		Lcd4_Write_String(temp1);
		    		P3_1?Lcd4_Write_String(" OFF"):Lcd4_Write_String(" ON ");//SE P3_1 FOR 1, POE OFF NO LCD. SE FOR 0, POE ON
		    	    Lcd4_Set_Cursor(2,0);
		    		sprintf(setpoint,"%03d",counter);//preenchimento de zeros para evitar uma contagem errada do sprintf
		    		Lcd4_Write_String("SP: ");
		    		Lcd4_Write_String(setpoint);
		    		  
		    		
		    		//PINO PARA ATIVACAO DO TRIAC OU OPTOACOPLADOR OU RELE DE ESTADO SOLIDO
		    		if(temperatura>counter)
		    		{
		    			P3_1=1;//PINO DO TRIAC. AQUI ELE DESLIGA O TRIAC
		    		}
		    		else
		    		{
		    		 	P3_1=0;
	    			}
	    		}
	    		else
	    		{
	    			if(flagTempDesl)
	    			{
		    			Lcd4_Clear();
		    			Lcd4_Set_Cursor(1,0);
		    			Lcd4_Write_String("Gire cursor do en");
		    			Lcd4_Set_Cursor(2,0);
		    			Lcd4_Write_String("cod. p/ muda vlr");
		    			Delay(1000);
		    			Lcd4_Clear();
		    			Lcd4_Set_Cursor(1,0);
		    			Lcd4_Write_String("PRESS. P/ SALVAR");
		    			Lcd4_Set_Cursor(2,0);
		    			Lcd4_Write_String("O NVO TEMP. DESL");
		    			Delay(1000);
		    			Lcd4_Clear();
	    			}
	    			flagTempDesl=0;
	    			Lcd4_Set_Cursor(1,0);
	    			Lcd4_Write_String("Tempo desl. ");
				    sprintf(tempDesl,"%03d",defTemp);
	    			Lcd4_Write_String(tempDesl);
	    		}
    		}
     
		    else
		    {
		        P3_1=1;// Desliga o TRIAC ou optoacoplador OU RELE DE ESTADO SOLIDO
		        Lcd4_Set_Cursor(1,0);
		        Lcd4_Write_String("Tempo Max de Uso");
		        Lcd4_Set_Cursor(2,0);
		        Lcd4_Write_String("atingido!");
		        Delay(1000);
		        Lcd4_Clear();
		        Lcd4_Set_Cursor(1,0);
		        Lcd4_Write_String("Pres. o botao do");
		        Lcd4_Set_Cursor(2,0);
		        Lcd4_Write_String("encod p/ reusar");
		        Delay(1000);
		        Lcd4_Clear();
		        }
		        //Botao do encoder na interrupcao externa 1 reseta o contador
		}
}


void TrataIntExt0(void) interrupt 0	// Interrupcao para o pino P3.2 - CLK do POT.ENC
{
	EX0=0;
    if(flagBoucing==1)	
	{ 
		opcao?counter++:defTemp++;
    }
    if(flagBoucing==2)
    { 		
        opcao?counter--:defTemp--;
    }
 	
 	(counter<200)?counter=200:(counter>400)?counter=400:counter; // Se counter for menor que 200, counter igual a 200, se nao--> se counter for maior que 400, counter igual 400, se nao counter eh o valor atual

 	(defTemp<2)?defTemp=2:(defTemp>240)?defTemp=240:defTemp;	//Se defTemp for menor que 2, defTemp==2, se nao-->se defTemp for maior que 240, defTemp==240. se nao defTemp eh igual ao valor que esta definido
	
	EX0=1;
	flagBoucing=0;
}

void Timer0() interrupt 1 // TIMER PARA COMPENSAR LEITURAS RAPIDAS DEMAIS DO ROTARY ENCODER
{
	
	TR0=0;
	TH0=0xFC;
	TL0=0x18;
	TR0=1;

	if(P3_0==0)
	{ 
		flagBoucing = 1;
	}
	else
	{
		flagBoucing = 2;	
	}

}

void Externa1(void) interrupt 2 // BOTAO SW DO ROTARY ENCODER PARA DAR RESET NO TEMPORIZADOR GERAL 
{
	EX1=0;
	temporizadorGeral=0;
	opcao^=1;//ALTERNA ENTRE ZERO E UM A VARIAVEL
	flagTempDesl=1;
    EX1=1;
}


void Timer1() interrupt 3 // TIMER CONTANDO DE 0uS ATE O MAXIMO DELE (65535uS) E INCREMENTANDO A VARIAVEL DE TEMPORIZADOR GERAL. 
{
	TR1=0;
	TH1=0x00;
	TL1=0x00;
	if(temporizadorGeral<tempoDeDesligamento(defTemp)+1)//O "+1" AQUI EH PARA GARANTIR QUE NO BLOCO PRINCIPAL DA APLICACAO, A CONDICIONAL RELACIONADA AO TEMPO GERAL DE FUNCIONAMENTO NAO SEJA EXECUTADA
        temporizadorGeral++;
 	
 	if(P3_1) //novo beep de setpoint sem delay
 	{
    	if(tempoBeep<17) //o 17 eh aproximadamente 1segundo
    	{
        	P3_4=0; //pino do beep. beep ligado.
        	tempoBeep++;
    	}
    	else
    	{
    		P3_4=1; // desligo o beep.
    	}
    }
    else
    {
    	tempoBeep=0;
    	P3_4=1; // garantindo que, caso a temperatura desca bruscamente, que o beep nao fique acionado direto.
    }

	TR1=1;
}

float readadc(bit d1,bit d0)	//LEITURA DO ADC MCP3204
{
	unsigned int adc_val=0;
	float temp;
	char i;
	cs=1;
	clk=1;mosi=1;
	cs=0;
	clk=0;clk=1;
	clk=0;clk=1;
	clk=0;clk=1;
	//LEMBRANDO QUE O MOSI JA ESTA 1!!! O MESMO FOI OMITIDO EM ALGUNS LOCAIS ABAIXO.VIDE DATASHEET
	clk=0;
	mosi=d1;
	clk=1;
	clk=0;
	mosi=d0;
	clk=1;
	clk=0;clk=1;
    clk=0;clk=1;
    
	for(i=11;i>=0;i--)
	{
		clk=0;

		if(miso)
		    adc_val|=(1<<i);
		    
		clk=1;
	}
	cs=1;
	//temp=((adc_val*5)/4095.0);       //((adc_val*5.0)/4095); 5V->VRef Max do ADC. O ".0" EH PARA NAO REALIZAR O TRUNCAMENTO DO NUMERO COMO UM INTEIRO, E SIM SER COMPREENDIDO COMO UM FLOAT
	temp=adc_val;
	return temp;
}


float conversaoTensaoEmTemperatura(float temperaturaEmVolts){
	//Supondo que o ganho seja de 100. VALOR DEVE SER ALTERADO DEPENDENDO DA CONFIGURACAO DO CIRCUITO DE AMPLIFICACAO. RESULTADO ABAIXO SEM A AMPLIFICACAO
	float ganhoDoAmplificador=100,
	tensaoEmMili = (temperaturaEmVolts/ganhoDoAmplificador)+0.00114,
	resultado; 
	//0.00114->1.14mV. Adicionado experimentalmente para adequar-se a temperatura ambiente. Medido pelo N1200 no modo "tipo K". Erro de 2 graus a 3 graus para menos. No entanto esse valor somado garante uma temperatura minima de 28 graus Celsius 
	//constantes de conversao de um termopar tipo K para transformar os milivolts em temperatura em celsius. 
	resultado=(0.226584602+(24152.10900*tensaoEmMili)+(67233.4248*(tensaoEmMili*tensaoEmMili))+(2210340.682*(tensaoEmMili*tensaoEmMili*tensaoEmMili))+(-860963914.9*(tensaoEmMili*tensaoEmMili*tensaoEmMili*tensaoEmMili))+(48350600000*(tensaoEmMili*tensaoEmMili*tensaoEmMili*tensaoEmMili*tensaoEmMili))+(-1184520000000*(tensaoEmMili*tensaoEmMili*tensaoEmMili*tensaoEmMili*tensaoEmMili*tensaoEmMili))+(13869000000000*(tensaoEmMili*tensaoEmMili*tensaoEmMili*tensaoEmMili*tensaoEmMili*tensaoEmMili*tensaoEmMili))+(-63370800000000*(tensaoEmMili*tensaoEmMili*tensaoEmMili*tensaoEmMili*tensaoEmMili*tensaoEmMili*tensaoEmMili*tensaoEmMili)));
	return resultado;
}

//TEMP = a0 + a1*uo + a2*uo2 + a3*uo3+ a4*uo4 + ... (u0/V, T/ oC) FORMULA NO RETURN ACIMA
/*
3- Tensao maxima do termopar. Vi na net que esse da Hikari(HK-P01) chega a 400 graus Celsius. Se for isso, provavelmente a tensao maxima dele eh de 16,397mV(vide Tabela do termopar tipo K)
4- Se a tensao maxima do termopar for 16.397mV(o que eu acho que eh msm, vi em umas 2 tabelas com exatamente o mesmo valor), entao temos que
fazer o ganho ser tal que os 16.397mV se transformem em 5V(q eh o maximo do ADC). Ou seja, o ganho eh de 304.933829359, aprox 305.
//RESISTORES PARA AMPLIFICADOR OPERACIONAL NO MODO AMPLIFICADOR NAO INVERSOR!!!!! R2==82K || R1==270R
*/