/*
 
 CS531 FINAL PROJECT - (WORK IN PROGRESS)
 
 SAMPLE DATA FROM ARDUINO TEST OUTPUT
 
 $GPRMC,232155.000,A,3854.7070,N,07728.5584,W,22.81,28.84,181115,,,A*70
 $GPGGA,232156.000,3854.7121,N,07728.5545,W,1,07,1.18,97.2,M,-33.4,M,,*61
 
 ----------------------------------------------------------------------------
 
 RECORD LAYOUT AND LINE SPECIFICATION FOR GPRMC GPS DATA (DATA TYPE 1)
 
 Example: $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
 
 Where:
 RMC          Recommended Minimum sentence C
 123519       Fix taken at 12:35:19 UTC
 A            Status A=active or V=Void.
 4807.038,N   Latitude 48 deg 07.038' N
 01131.000,E  Longitude 11 deg 31.000' E
 022.4        Speed over the ground in knots
 084.4        Track angle in degrees True
 230394       Date - 23rd of March 1994
 003.1,W      Magnetic Variation
 *6A          The checksum data, always begins with *
 
 
 $GPRMC,hhmmss.ss,A,llll.ll,a,yyyyy.yy,a,x.x,x.x,ddmmyy,x.x,a*hh
 
 1    = UTC of position fix
 2    = Data status (V=navigation receiver warning)
 3    = Latitude of fix
 4    = N or S
 5    = Longitude of fix
 6    = E or W
 7    = Speed over ground in knots
 8    = Track made good in degrees True
 9    = UT date
 10   = Magnetic variation degrees (Easterly var. subtracts from true course)
 11   = E or W
 12   = Checksum
 
 ----------------------------------------------------------------------------
 
 RECORD LAYOUT AND LINE SPECIFICATION FOR GPGGA DATA (DATA TYPE 2)
 
 GGA - essential fix data which provide 3D location and accuracy data.
 
 $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
 
 Where:
 GGA          Global Positioning System Fix Data
 123519       Fix taken at 12:35:19 UTC
 4807.038,N   Latitude 48 deg 07.038' N
 01131.000,E  Longitude 11 deg 31.000' E
 1            Fix quality: 0 = invalid
 1 = GPS fix (SPS)
 2 = DGPS fix
 3 = PPS fix
 4 = Real Time Kinematic
 5 = Float RTK
 6 = estimated (dead reckoning) (2.3 feature)
 7 = Manual input mode
 8 = Simulation mode
 08           Number of satellites being tracked
 0.9          Horizontal dilution of position
 545.4,M      Altitude, Meters, above mean sea level
 46.9,M       Height of geoid (mean sea level) above WGS84 ellipsoid
 (empty field) time in seconds since last DGPS update
 (empty field) DGPS station ID number
 *47          the checksum data, always begins with *
 
 ----------------------------------------------------------------------------
 
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#define MAXREAD 100


// GPS DATA TYPE 1 CONTAINING LAT & LONG, DATE, TIME, AND OTHER PROPERTIES

struct gprmc {
    
    const char *gps_reading_type;
    float fix_time;
    char status;
    float latitude;
    char lat_direction;
    float longitude;
    char long_direction;
    float speed;
    float track_angle;
    int date;
    float mag_var;
    char mag_var_direction;
    char *checksum;
    
};


// GPS DATA TYPE 2 CONTAINING ADDITIONAL METADATA

struct gpgga {
    
    const char *gps_reading_type;
    float fix_time;
    float latitude;
    char lat_direction;
    float longitude;
    char long_direction;
    int fix_qual;
    int number_of_sats;
    float horiz_dilution;
    float altitude;
    char alt_measure_type;
    float height;
    char height_measure_type;
    char *elapsed_fix_time_gps_station_id;
    char *checksum;
    
};


// ARRAY OF STRUCTURE POINTERS TO HOLD GPS DATA LOADED INTO STRUCTURES

struct gprmc *gpsDataType1[MAXREAD];
struct gpgga *gpsDataType2[MAXREAD];


// FUNCTION PROTOTYPES TO PARSE & LOAD LINE DATA OF THE TWO GPS DATA TYPES

struct gprmc *loadGPRMCData(char *line);
struct gpgga *loadGPGGAData(char *line);
struct tm  * convertDateTime(float utc, int date);


void *print_html(void *arg)
{
    
    FILE *fp;
    fp = fopen("test.html","w"); // write mode
    
    if( fp == NULL )
    {
        perror("Error creating the file.\n");
        exit(EXIT_FAILURE);
    }
   
       /* NEEDS DEVELOPMENT RIGHT HERE TO INTRODUCE THE HTML HEADERS, TAGS, AND ANY JAVASCRIPT  */

    printf("***** T E S T *****\n\n");

    
    /* TEST FOR PRINTING FIELDS - ONLY PULLS OUT DATA FROM THE $GPRMC STRUCTURE */
    
        for (int i = 0; gpsDataType1[i] != '\0'; i++) {
    
            struct tm * date_time = convertDateTime(gpsDataType1[i]->fix_time,gpsDataType1[i]->date);
            
                printf("Structure GPRMC # %d:\n Date-> %d \n Time (UTC)-> %.2f \n Latitude-> %.4f \n Latitude Direction-> %c \n Longitude-> %.4f \n Longitude Direction-> %c \n\n Converted Date Time: %d/%d/%d %d:%d:%d \n\n",
                                   i,
                                   gpsDataType1[i]->date,
                                   gpsDataType1[i]->fix_time,
                                   gpsDataType1[i]->latitude,
                                   gpsDataType1[i]->lat_direction,
                                   gpsDataType1[i]->longitude,
                                   gpsDataType1[i]->long_direction,
                                   date_time->tm_mon, date_time->tm_mday, date_time->tm_year,date_time->tm_hour, date_time->tm_min, date_time->tm_sec);
    
                free(date_time);
    
        }
    
    return NULL;
}



int main(int argc, char **argv) {
    
    
    // LOAD ARDUINO GPS DATA FILE FROM COMMAND LINE ARGUMENT
    
    char *file_name = argv[1];
    
    FILE *fp;
    fp = fopen(file_name,"r"); // read mode
    
    if( fp == NULL )
    {
        perror("Error while opening the file.\n");
        exit(EXIT_FAILURE);
    }
    
 
    // CREATE PRINTING THREAD
    
    pthread_t print_html_thread;

    if(pthread_create(&print_html_thread, NULL, print_html,NULL)) {
        
        perror("Error creating thread\n");
        exit(EXIT_FAILURE);
        
    }
    
  
    
    /*
     
     READ LINE DATA IN AND DETERMINE WHICH TYPE OF GPS DATA IT IS. INSPECT THE READ LINE AND READ THE FIRST SEVEN
     CHARS TO DETERMINE WHICH STRUCTURE TO LOAD. THE DATA ALTERNATES BETWEEN $GPRMC AND$GPGGA DATA TYPES
     
     */
    
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int line_num = 0;
    char peek[8];
    
    while ((read = getline(&line, &len, fp)) != -1) {
        
        
        
        strncpy(peek, line, 7);
        
        if(strcmp(peek, "$GPRMC," ) == 0)
            
            gpsDataType1[line_num++] =  loadGPRMCData(line);
        
        else if (strcmp(peek, "$GPGGA," ) == 0) {
            
            gpsDataType2[--line_num] =  loadGPGGAData(line);
            ++line_num;
   	    }
        else {
            
            perror("Error. GPS data specification not found.");
            exit(EXIT_FAILURE);
        }
    }
 
    pthread_join(print_html_thread, NULL);

    fclose(fp);
    
    pthread_exit(NULL);

    
    return 0;
    

}



/* PARSER FOR $GPRMC LINE OF DATA. LOADS THE PARSED COMMA SEPARATED TOKENS INTO THE DESIGNATED STRUCTURE  */

struct gprmc *loadGPRMCData(char *line) {
    
    struct gprmc *p = malloc(sizeof(struct gprmc));
    
    char *token;
    int token_num = 0;
    
    while ((token = strsep(&line, ",")) != NULL) {
        
        //       printf("%s\n", token);
        
        switch(token_num) {
                
            case 0:
                p->gps_reading_type = token;
                token_num++;
                break;
            case 1:
                p->fix_time =  atof(token);
                token_num++;
                break;
            case 2:
                p->status =  *token;
                token_num++;
                break;
            case 3:
                p->latitude = atof(token);
                token_num++;
                break;
            case 4:
                p->lat_direction = *token;
                token_num++;
                break;
            case 5:
                p->longitude =  atof(token);
                token_num++;
                break;
            case 6:
                p->long_direction =  *token;
                token_num++;
                break;
            case 7:
                p->speed =  atof(token);
                token_num++;
                break;
            case 8:
                p->track_angle =  atof(token);
                token_num++;
                break;
            case 9:
                p->date =  atoi(token);
                token_num++;
                break;
            case 10:
                p->mag_var =  atof(token);
                token_num++;
                break;
            case 11:
                p->mag_var_direction =  *token;
                token_num++;
                break;
            case 12:
                p->checksum =  token;
                token_num++;
                break;
            default:
                perror("Error. Additional token found.\n");
                exit(EXIT_FAILURE);
        }
    }
    return p;
}

/* PARSER FOR $GPGGA LINE OF DATA. LOADS THE PARSED COMMA SEPARATED TOKENS INTO THE DESIGNATED STRUCTURE  */

struct gpgga *loadGPGGAData(char *line) {
    
    struct gpgga *p = malloc(sizeof(struct gpgga));
    
    char *token;
    int token_num = 0;
    
    while ((token = strsep(&line, ",")) != NULL) {
        
        //        printf("%s\n", token);
        
        switch(token_num) {
                
            case 0:
                p->gps_reading_type = token;
                token_num++;
                break;
            case 1:
                p->fix_time =  atof(token);
                token_num++;
                break;
            case 2:
                p->latitude = atof(token);
                token_num++;
                break;
            case 3:
                p->lat_direction = *token;
                token_num++;
                break;
            case 4:
                p->longitude =  atof(token);
                token_num++;
                break;
            case 5:
                p->long_direction =  *token;
                token_num++;
                break;
            case 6:
                p->fix_qual =  atoi(token);
                token_num++;
                break;
            case 7:
                p->number_of_sats =  atoi(token);
                token_num++;
                break;
            case 8:
                p->horiz_dilution =  atof(token);
                token_num++;
                break;
            case 9:
                p->altitude =  atof(token);
                token_num++;
                break;
            case 10:
                p->alt_measure_type =  *token;
                token_num++;
                break;
            case 11:
                p->height =  atof(token);
                token_num++;
                break;
            case 12:
                p->height_measure_type =  *token;
                token_num++;
                break;
            case 13:
                p->elapsed_fix_time_gps_station_id =  token;
                token_num++;
                break;
            case 14:
                p->checksum =  token;
                token_num++;
                break;
            default:
                perror("Error. Additional token found.\n");
                exit(EXIT_FAILURE);
        }
    }
    return p;
    
}

/* FUNCTION TO CREATE A DATE TIME STAMP STRUCTURE FOR FURTHER USE */
struct tm  * convertDateTime(float utc, int date){
    
    struct tm *p = malloc(sizeof(struct tm));
    
    int time = (int) utc;
    
    p->tm_hour = (time / 10000) % 100;
    p->tm_min = (time / 100) % 100;
    p->tm_sec = time % 100;
    p->tm_mday = (date / 10000) % 100;
    p->tm_mon = (date / 100) % 100;
    p->tm_year = (date % 100) + 2000;
    
    
    return p;
}


    
    


