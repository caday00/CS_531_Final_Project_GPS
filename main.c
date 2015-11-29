/*
 CS531 FINAL PROJECT - (WORK IN PROGRESS)
 
 SAMPLE DATA FROM ARDUINO TEST OUTPUT
 
 $GPRMC,232155.000,A,3854.7070,N,07728.5584,W,22.81,28.84,181115,,,A*70
 
 ----------------------------------------------------------------------------
 
 RECORD LAYOUT AND LINE SPECIFICATION FOR GPRMC GPS DATA
 
 Example: $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W,*6A
 
 Where:
    Field #     Value        Description
    1           $GPRMC       Recommended Minimum sentence C
    2           123519       Position Fix taken at 12:35:19 UTC
    3           A            Data Status A=active or V=Void (navigation receiver warning)
    4           4807.038     Latitude of fix: 48 deg 07.038'
    5           N            Latitude direction: N  (N or S)
    6           01131.000    Longitude of fix: 11 deg 31.000'
    7           E            Longitude direction: E (E or W)
    8           022.4        Speed over the ground in knots
    9           084.4        Track angle in degrees True
    10          230394       Date - 23rd of March 1994
    11          003.1        Magnetic variation degrees
    12          W            Magnetic variation direction (E or W)
    13          *6A          Checksum data, always begins with *
 
 ---------------------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <math.h>

#define MAXREAD 10000

// GPS DATA TYPE (RMC) CONTAINING LAT & LONG, DATE, TIME, AND OTHER PROPERTIES
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

// FUNCTION PROTOTYPE TO PARSE & LOAD LINE DATA OF THE GPS DATA
struct gprmc *loadGPRMCData(char *);
struct tm  * convertDateTime(float, int);
float convertDegreeToDecimal(float, char);

// FUNCTION PROTOTYPE TO PRINT THE GPS DATA TO FILE AND FORMAT FOR HTML DISPLAY
void *print_html(void *);


// FREE STRUCT POINTERS FROM MEMORY
void clean();

int main(int argc, char **argv) {
    // LOAD ARDUINO GPS DATA FILE FROM COMMAND LINE ARGUMENT
    char *file_name = argv[1];
    
    FILE *fp;
    fp = fopen(file_name,"r");
    
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
     READ LINE DATA IN AND DETERMINE WHICH TYPE OF GPS DATA IT IS BY INSPECTING THE FIRST SEVEN CHARS.
     */
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int line_num = 0;
    char peek[6];
    
    while ((read = getline(&line, &len, fp)) != -1) {
        strncpy(peek, line, 6);
        if(strcmp(peek, "$GPRMC" ) == 0)
            gpsDataType1[line_num++] =  loadGPRMCData(line);
        else {
            perror("Error. GPS data specification not found.");
            exit(EXIT_FAILURE);
        }
    }
    
    pthread_join(print_html_thread, NULL);
    fclose(fp);
    clean();
    pthread_exit(NULL);
    
    return 0;
}

/* PARSER FOR $GPRMC LINE OF DATA. LOADS THE PARSED COMMA SEPARATED TOKENS INTO THE DESIGNATED STRUCTURE  */
struct gprmc *loadGPRMCData(char *line) {
    struct gprmc *p = malloc(sizeof(struct gprmc));
    char *token;
    int token_num = -1;
    
    while ((token = strsep(&line, ",")) != NULL) {
        switch(++token_num) {
            case 0:
                p->gps_reading_type = token;
                break;
            case 1:
                p->fix_time = atof(token);
                break;
            case 2:
                p->status = *token;
                break;
            case 3:
                p->latitude = atof(token);
                break;
            case 4:
                p->lat_direction = *token;
                break;
            case 5:
                p->longitude = atof(token);
                break;
            case 6:
                p->long_direction = *token;
                break;
            case 7:
                p->speed = atof(token);
                break;
            case 8:
                p->track_angle = atof(token);
                break;
            case 9:
                p->date = atoi(token);
                break;
            case 10:
                p->mag_var = atof(token);
                break;
            case 11:
                p->mag_var_direction = *token;
                break;
            case 12:
                p->checksum = token;
                break;
            default:
                perror("Error. Additional token found.\n");
                exit(EXIT_FAILURE);
        }
    }
    return p;
}

/* FUNCTION EXCUTED BY THREAD TO CREATE AN HTML FILE & DISPLAY A TABLE OF GPS DATA AND HYPERLINK TO MAP LOCATION  */
void *print_html(void *arg){
    FILE *fp;
    fp = fopen("test.html","w");
    
    if( fp == NULL )
    {
        perror("Error creating the file.\n");
        exit(EXIT_FAILURE);
    }
    
    fprintf(fp, "<!DOCTYPE html>\n");
    fprintf(fp, "<html>\n");
    fprintf(fp, "<body>\n");
    fprintf(fp, "<meta charset=\"UTF-8\">");
    fprintf(fp, "\t<table style=\"width:100%%\">");
    fprintf(fp, "\t<tr>");
    fprintf(fp, "\t<th>Row</th>\n\t<th>Fix Time</th>\n\t<th>Status</th>\n\t<th>Latitude</th>\n\t<th>Lat Direction</th>\n\t<th>Longtitude</th>\n\t<th>Long Direction</th>\n\t<th>Date/Time</th>\n\t<th>Link to Map</th>");
    fprintf(fp, "</tr>");
    
    
    for (int i = 0; gpsDataType1[i] != '\0'; i++) {
        struct tm * date_time = convertDateTime(gpsDataType1[i]->fix_time,gpsDataType1[i]->date);
        fprintf(fp, "\t<tr><td><center>%d</center></td><td><center>%.4f</center></td><td><center>%c</center></td><td><center>%.4f</center></td><td><center>%c</center></td><td><center>%.4f</center></td><td><center>%c</center></td><td><center>%d/%d/%d %d:%d:%d</center></td><td><center><a href=\"https://www.google.com/maps/?q=%.4f,%.4f\" target=\"_blank\">Click</a></center></td></tr>",
                
                i,
                gpsDataType1[i]->fix_time,
                gpsDataType1[i]->status,
                gpsDataType1[i]->latitude,
                gpsDataType1[i]->lat_direction,//N or S
                gpsDataType1[i]->longitude,
                gpsDataType1[i]->long_direction, //E or W
                date_time->tm_mon, date_time->tm_mday, date_time->tm_year,date_time->tm_hour, date_time->tm_min, date_time->tm_sec, convertDegreeToDecimal(gpsDataType1[i]->latitude,gpsDataType1[i]->lat_direction), convertDegreeToDecimal(gpsDataType1[i]->longitude, gpsDataType1[i]->long_direction));
        
        free(date_time);
    }
    fprintf(fp, "\n\t</table>");
    fprintf(fp, "\n</body>");
    fprintf(fp, "\n</html>");
    fclose(fp);
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


/* FUNCTION TO CONVERT THE PARSED GPS COORDINATE DEGREE MINUTE DATA AND CONVERT TO DEGREE DECIMAL VALUE */
float convertDegreeToDecimal(float value, char direction){
    
    //Decimal Degrees = Degrees + minutes/60 + seconds/3600
    
    int sign;
    int degrees;
    float converted_minutes;
    
    switch (direction) {
        case 'N':
        case 'E':
            sign = 1;
            break;
        case 'S':
        case 'W':
            sign = -1;
            break;
        default:
            perror("No valid direction found.");
            break;
    }
    
    degrees = (int) value / 100;
    converted_minutes = fmodf(value,100) / 60;
    
    return sign * (degrees + converted_minutes);
}


/* FREE-UP MEMORY USED BY GPS STRUCTURE  */
void clean(){
    
    for (int i = 0; gpsDataType1[i] != '\0'; i++)
        free(gpsDataType1[i]);
    
}

