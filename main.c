/*
 CS531 FINAL PROJECT - (WORK IN PROGRESS)
 
 SAMPLE DATA FROM ARDUINO TEST OUTPUT
 
 $GPRMC,232155.000,A,3854.7070,N,07728.5584,W,22.81,28.84,181115,,,A*70
 
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
 ---------------------------------------------------------------------------------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#define MAXREAD 10000

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

// ARRAY OF STRUCTURE POINTERS TO HOLD GPS DATA LOADED INTO STRUCTURES
struct gprmc *gpsDataType1[MAXREAD];

// FUNCTION PROTOTYPES TO PARSE & LOAD LINE DATA OF THE TWO GPS DATA TYPES
struct gprmc *loadGPRMCData(char *line);
struct tm  * convertDateTime(float utc, int date);
void *print_html(void *arg);

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
        switch(token_num) {                
            case 0:
                p->gps_reading_type = token;
                token_num++;
                break;
            case 1:
                p->fix_time = atof(token);
                token_num++;
                break;
            case 2:
                p->status = *token;
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
                p->longitude = atof(token);
                token_num++;
                break;
            case 6:
                p->long_direction = *token;
                token_num++;
                break;
            case 7:
                p->speed = atof(token);
                token_num++;
                break;
            case 8:
                p->track_angle = atof(token);
                token_num++;
                break;
            case 9:
                p->date = atoi(token);
                token_num++;
                break;
            case 10:
                p->mag_var = atof(token);
                token_num++;
                break;
            case 11:
                p->mag_var_direction = *token;
                token_num++;
                break;
            case 12:
                p->checksum = token;
                token_num++;
                break;
            default:
                perror("Error. Additional token found.\n");
                exit(EXIT_FAILURE);
        }
    }
    return p;
}

void *print_html(void *arg){
    FILE *fp;
    fp = fopen("test.html","w"); // write mode
    
    if( fp == NULL )
    {
        perror("Error creating the file.\n");
        exit(EXIT_FAILURE);
    }
    fprintf(fp, "<body>\n");
	fprintf(fp, "<meta charset=\"UTF-8\">");
	fprintf(fp, "\t<table style=\"width:100%\">");
	fprintf(fp, "\t<tr>");
	fprintf(fp, "\t<th>Row</th>\n\t<th>Fix Time</th>\n\t<th>Status</th>\n\t<th>Latitude</th>\n\t<th>Lat Direction</th>\n\t<th>Longtitude</th>\n\t<th>Long Direction</th>\n\t<th>Date/Time</th>");
	fprintf(fp, "</tr>");
    
  	int i = 0;
	for (i; gpsDataType1[i] != '\0'; i++) {
            struct tm * date_time = convertDateTime(gpsDataType1[i]->fix_time,gpsDataType1[i]->date);
            fprintf(fp, "\t<tr><td><center>%d</center></td><td><center>%.4f</center></td><td><center>%c</center></td><td><center>%.4f</center></td><td><center>%c</center></td><td><center>%.4f</center></td><td><center>%c</center></td><td><center>%d/%d/%d %d:%d.%d</center></td></tr>",
                                   i,
                                   gpsDataType1[i]->fix_time,
								   gpsDataType1[i]->status,
                                   gpsDataType1[i]->latitude, 
                                   gpsDataType1[i]->lat_direction,//N or S
                                   gpsDataType1[i]->longitude, 
                                   gpsDataType1[i]->long_direction, //E or W
                                   date_time->tm_mon, date_time->tm_mday, date_time->tm_year,date_time->tm_hour, date_time->tm_min, date_time->tm_sec);
                free(date_time);
        }	
	fprintf(fp, "\n\t</table>");
	fprintf(fp, "\n</body>");    
    return NULL;
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
