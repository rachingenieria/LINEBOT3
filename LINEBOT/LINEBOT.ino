//RACHINGENIERIA
//09 24 2024
//VERSION PROBADA

#include <stdio.h>
#include <string.h>

#include "eeprom.h"
#include "motores.h"
#include "linea.h"
#include "api.h"
#include "rachvel.h"

#include <Servo.h>

Servo myservo;  // create Servo object to control a servo
int pos = 0;    // variable to store the servo position

//------------------------------------------------------------------------------------//
int error[10];
extern int sensores_b;
unsigned int sensorValues[NUM_SENSORS];
//--------------------------------------------------------------------------------------//
//PARAMETROS del Control del Velocista
//AQUI SE MODIFICAN LOS PARAMETROS DE COMPETENCIA
rachvel Rachvel;

 
int   VELCIDAD_MAXIMA        = 30;       //Velocidad Maxima (entre 0 y 100)
int   CTE_PROPORCIONAL       = 7;      //Constante de Control Proporcional (ente 1 y 20)
int   CTE_DERIVATIVA         = 30;      //Constante de Control Diferencia (ente 1 y 20)
int   V_TURBINA              = 30;      //Constante de Control Diferencia (ente 1 y 20)
                                                                                                                                                                                                                            
int   PISTACOLOR             = 0;


//Variables para Control Adicional
float power_difference, power_difference_ext;
float power_difference_ant;
int detect_recta_ant, detect_recta;
char stat_sw = 0; 
 
void setup()
{
  int val;  //Variable para leer el estado del pulsador

  //myservo.attach(5);  // attaches the servo on pin 9 to the Servo object
  
  pinMode(LED2, OUTPUT);
  pinMode(LED1, OUTPUT);
  
  pinMode(SW1,INPUT);
  digitalWrite(SW1, HIGH); //PULL UP 
  
  pinMode(ON_RF,INPUT); 
  digitalWrite(ON_RF, LOW); //PULL DOWN
   
  Serial.begin(9600);

  Motor_Init();
  SetSpeeds(0,0);

  Eeprom_read();
  if(Rachvel.ver == FIRMWARE_VERSION)// DATOS CORRECTOS y CARGADOS
  {    
    ; // DATOS GUARDADOS EN LA FLASH DEL MICROCONTROLADOR
  }
  else
  {
    Rachvel.setupconfig(VELCIDAD_MAXIMA,CTE_PROPORCIONAL,CTE_DERIVATIVA,V_TURBINA); //Cargar valores por DEFECTO
    Eeprom_save(); //SALVAR EL LA FLASH
  }

  
  int cursorz = 0;
  int menuactivo = 0;
  
    // MENU DE CONFIGURACION
    //Serial.println("RECIBIR DATOS POR BLUETOOTH");
    Serial_send_variables();


  digitalWrite(LED2, HIGH);
  digitalWrite(LED1, HIGH);
  delay(500);
  digitalWrite(LED2, LOW);
  digitalWrite(LED1, LOW);
  
  SetSpeeds(0, 0); 
  val = 1;
  int mx = 0;
  int my = 0;
  Rachvel.mx = 0;
  Rachvel.my = 0;
  
  do{
      //RECIBIR DATOS POR BLUETOOTH
      Serial_command();
      val = digitalRead(SW1);  
      
         mx =  -(Rachvel.vavg * Rachvel.mx)/10;
         my =   (Rachvel.vavg * Rachvel.my)/10;
      
      SetSpeeds(mx + my, mx - my);
      
  }while (val);
  

 SetSpeeds(0, 0); 

  digitalWrite(LED2, LOW);
  digitalWrite(LED1, LOW);
  delay(200);
  digitalWrite(LED2, HIGH);     
  digitalWrite(LED1, HIGH);
  delay(200);
  digitalWrite(LED2, LOW);     
  digitalWrite(LED1, LOW);
  delay(200);                  
  digitalWrite(LED2, HIGH);     
  digitalWrite(LED1, HIGH);   
  delay(200);
  digitalWrite(LED2, LOW);     
  digitalWrite(LED1, LOW); 
  

//-------------Instrucciones para Empezar a hacer la Calibracion de Sensores--------------------------------------//
  
  Reset_Calibracion(); //ROBOT EN MEDIO DE LA LINEAS
  Rachvel.colorlinea = Calibrar_Color_Linea(sensorValues);
  digitalWrite(LED2, LOW);
  digitalWrite(LED1, HIGH);
  
  //GIRA MIENTRA CALIBRA
  SetSpeeds(-250, 250);
  int tiempo_cal = NUM_MUESTRAS + 1;
  while(tiempo_cal--)
  {
      Calibrar_Sensores(sensorValues);
      delay(5);
  }
  digitalWrite(LED2, HIGH);
  digitalWrite(LED1, LOW);
  SetSpeeds(0, 0);

  val = digitalRead(SW1);  
  Rachvel.position_line = 60;
  
  while (val == HIGH )
          {
             val = digitalRead(SW1);  
             Rachvel.position_line = Leer_linea(sensorValues,Rachvel.position_line ,Rachvel.colorlinea, 5 ); // leemos posicion de la linea en la variable position
          
             
               if (Rachvel.position_line < -20)
               {
                   digitalWrite(LED2, LOW);
                   digitalWrite(LED1, HIGH); 
                }
                else  if (Rachvel.position_line > 20 )
                {
                   digitalWrite(LED2, HIGH);
                   digitalWrite(LED1, LOW);    
                }
          
                error[5]=error[4];
                error[4]=error[3];
                error[3]=error[2];
                error[2]=error[1];
                error[1]=error[0];
                error[0]=Rachvel.position_line;
          
                power_difference = (error[0] * Rachvel.kpg) + ((error[0] - error[4]) * Rachvel.kdg);
                SetSpeeds( - power_difference,  power_difference);
              
                delay(1);
}
  
   //---------------------------FIN DE PRUEBA DE CALIBRACION----------------------------------------------------//
   //stop Motors
  SetSpeeds(0,0);
  
  digitalWrite(LED2, LOW);
  digitalWrite(LED1, LOW);
  delay(200);
  digitalWrite(LED2, HIGH);     
  digitalWrite(LED1, HIGH);
  delay(200);
  digitalWrite(LED2, LOW);     
  digitalWrite(LED1, LOW);
  delay(200);                  
  digitalWrite(LED2, HIGH);     
  digitalWrite(LED1, HIGH);   
  delay(200);
  digitalWrite(LED2, LOW);     
  digitalWrite(LED1, LOW);
  
 //---------------------------Verificacion de Sensores----------------------------------------------------//
  val = digitalRead(SW1);  
  while (val == HIGH )
  {    
     Serial_send_variables();
     Rachvel.position_line = Leer_linea(sensorValues,Rachvel.position_line ,Rachvel.colorlinea, 5 ); 
     delay(100);
     val = digitalRead(SW1); 
  }

 //---------------------------LISTO PARA COMPETIR----------------------------------------------------// 
 //---------------------------SELECIONAR METODO DE INICO----------------------------------------------------//
  
  digitalWrite(LED2, LOW);
  digitalWrite(LED1, LOW);
  delay(200);
  digitalWrite(LED2, HIGH);     
  digitalWrite(LED1, HIGH);
  delay(200);
  digitalWrite(LED2, LOW);     
  digitalWrite(LED1, LOW);
  delay(200);                  
  digitalWrite(LED2, HIGH);     
  digitalWrite(LED1, HIGH);   
  delay(200);
  digitalWrite(LED2, LOW);     
  digitalWrite(LED1, LOW);
  delay(200);                  
  digitalWrite(LED2, HIGH);     
  digitalWrite(LED1, HIGH); 
 

// INICIO CON MODULO REMOTO
  
   Rachvel.start = 0; //STOP BLUETOOTH
   int rf_control = digitalRead(ON_RF);

  digitalWrite(LED2, HIGH); //ENCIENDE LEDS
  digitalWrite(LED1, HIGH); 
  val = 0;  
  
   while(rf_control == 0 && Rachvel.start == 0 && stat_sw == 0)
   {
      rf_control = digitalRead(ON_RF);
      Serial_command();
      delay(2);
      val = digitalRead(SW1); 
      if(val == LOW)
      {
          digitalWrite(LED2, HIGH);
          digitalWrite(LED1, HIGH);
          delay(1000);
          digitalWrite(LED2, HIGH);
          digitalWrite(LED1, LOW);
          delay(1000);
          digitalWrite(LED2, LOW);
          digitalWrite(LED1, HIGH);
          delay(1000);
          digitalWrite(LED2, HIGH);
          digitalWrite(LED1, HIGH);
          delay(1000);
          digitalWrite(LED2, LOW);
          digitalWrite(LED1, LOW);
          delay(999);
          stat_sw =  1;
      }
   }


  if(Rachvel.start == 0xFF) // MODULO CONECTADO PERO ARRANCA POR SW
  {
      stat_sw =  1; //NO PARA POR MODULO
  }
  
  if(rf_control) // MODULO NO CONECTADO, ARRANCA POR SW
  {
    stat_sw =  0; //NO PARA POR MODULO
  }

   Rachvel.start = 0xFF; //START BLUETOOTH
  
}
 
void loop()
{
 
//APAGADO POR MODULO REMOTO
   int rf_control = digitalRead(ON_RF);
   if (rf_control == 0 && stat_sw == 0)
   {
       SetSpeeds(0, 0);
       while(1)
       {
          digitalWrite(LED2, HIGH);
          digitalWrite(LED1, HIGH);
          delay(500);
          digitalWrite(LED2, LOW);
          digitalWrite(LED1, LOW);
          delay(500);
        
        }
   } // STOP ROBOT

   if (Rachvel.start == 0 && stat_sw == 1)
   {
       SetSpeeds(0, 0);
   }
   
  Rachvel.position_line = Leer_linea(sensorValues,Rachvel.position_line ,Rachvel.colorlinea, 5 ); // leemos posicion de la linea en la variable position

   
 if (Rachvel.position_line < -20)
 {
     digitalWrite(LED2, LOW);
     digitalWrite(LED1, HIGH); 
  }
  else  if (Rachvel.position_line > 20 )
  {
     digitalWrite(LED2, HIGH);
     digitalWrite(LED1, LOW);    
  }


  error[9]=error[8];
  error[8]=error[7];
  error[7]=error[6];
  error[6]=error[5];
  error[5]=error[4];
  error[4]=error[3];
  error[3]=error[2];
  error[2]=error[1];
  error[1]=error[0];
  error[0]=Rachvel.position_line;

 detect_recta_ant = detect_recta;
 detect_recta = 1;

      for(int i=0 ; i<10; i++)
      {
        if( error[i] > 8 || error[i] < -8)
        {
          detect_recta = 0;
        }

      }

      if(detect_recta) //es RECTA
      {
         digitalWrite(LED2, HIGH);
         digitalWrite(LED1, HIGH);
      }
      else //ES CURVA
      {
         if(detect_recta_ant) //ENTRA A CURVA
         {
             digitalWrite(LED2, LOW);
             digitalWrite(LED1, LOW);
             SetSpeeds(-900, -900);
             delay(5);
         }
      }


  int vavg= Rachvel.vavg;
  vavg = vavg*10;
  power_difference = (error[0] * Rachvel.kpg) + ((error[0] - error[4]) * Rachvel.kdg);

  if( Rachvel.start )
  {
     if(power_difference > 0)
      {
        SetSpeeds(vavg  - power_difference, vavg );
      }
      else if(power_difference < 0)
      {
        SetSpeeds(vavg , vavg +  power_difference);
      }
      else
      {
        SetSpeeds(vavg , vavg );
      }

  }
  else
  {
     SetSpeeds(0, 0);
  }
  
  //SERIAL STOP
   Serial_command();

   delay(1);
}
