#include <stdio.h>
#define MONEYCONVERT	1.2686
#define WEIGHTCONVERT	453.59237
#define DISTANCECONVERT 0.6213712

//Euro = 1.2686*USD
//Celsius = (5/9)(F-32)
//Pound = 453.59237 grams
//Kilomter = 0.6213712 miles
//Ireland Time = Austin + 6

int TimeConversion(int hour, int min, int choice){
	int finalhour; //This is the output variable
	if (choice == 0){
		finalhour = (hour + 6)%24;
		printf("%d ", finalhour);
		printf("%d is the time in Dublin.\n", min);
	}
	else {
		if(hour<6){
			finalhour = (hour + 18)%24;
		}
		else{
			finalhour = (hour - 6)%24;
		}
		printf("%d ", finalhour);
		printf("%d is the time in Austin.\n", min);
	}
	return 0;
}

double USDtoEUR(int dollar, int cent){
	double USD;
	double EUR;
	USD = cent + (dollar*100);
	EUR = MONEYCONVERT * USD;
}

int EURtoUSD(double EURamount){

}

double FtoC(int Faren){

}

int CtoF(double Celsius){

}

int KGtoPO(double kg){

}

double POtoKG(int pound, int ounce){

}

double Distance(int amount, int choice){

}


int main(void){
	int MenuOption=0;
	int hour;
	int min;
	int dollar;
	int cent;
	double euro;
	int faren;
	double celsius;
	double kg;
	int pound;
	int ounce;
	int distance;
	printf("Please choose an option:\n"
	"1. Convert Austin time to Dublin time.\n"
	"2. Convert Dublin time to Austin time.\n"
	"3. Convert USD value to EUR value.\n"
	"4. Convert EUR value to USD value.\n"
	"5. Convert Fahrenheit to Celsius.\n"
	"6. Convert Celsius to Fahrenheit.\n"
	"7. Convert kg to pounds , ounces.\n"
	"8. Convert pounds , ounces to kg.\n"
	"9. Convert km to miles.\n"
	"10. Convert miles to km.\n"
	"11. Stop conversions and quit the program.\n\n"
	);
	scanf("%d", & MenuOption);
	switch (MenuOption){
		case 1:
			printf("Please enter the hour and minute of the time\n");
			scanf("%d", & hour);
			scanf("%d", & min);
			TimeConversion(hour, min, 0);
			break;
		case 2:
			printf("Please enter the hour and minute of the time\n");
			scanf("%d", & hour);
			scanf("%d", & min);
			TimeConversion(hour, min, 1);
			break;
		case 3:
			printf("Please enter the dollar and cent amounts\n");
			scanf("%d", & dollar);
			scanf("&d", & cent);
			USDtoEUR(dollar, cent);
			break;
		case 4:
			printf("Please enter the amount of Euros\n");
			scanf("%d", & euro);
			EURtoUSD(euro);
			break;
		case 5:
			printf("Please enter the degrees in Farenheit\n");
			scanf("%d", & faren);
			FtoC(faren);
			break;
		case 6:
			printf("Please enter the degrees in Celsius\n");
			scanf("%d", & celsius);
			CtoF(celsius);
			break;
		case 7:
			printf("Please enter the amount of kilograms\n");
			scanf("%d", & kg);
			KGtoPO(kg);
			break;
		case 8:
			printf("Please enter the amount of pounds and ounces\n");
			scanf("%d", & pound);
			scanf("%d", & ounce);
			POtoKG(pound, ounce);
			break;
		case 9:
			printf("Please enter the amount of kilometers\n");
			scanf("%d", & distance);
			Distance(distance, 0);
			break;
		case 10:
			printf("Please enter the amount of miles\n");
			scanf("%d", & distance);
			Distance(distance, 1);
			break;
		case 11:
			
			break;
	}
	return 0;
}