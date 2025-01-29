/*
 * AP_PDRL_Commander_Logger.cpp
 *
 *  Created on: 31-Oct-2019
 *      Author: owner
 */
#include <AP_HAL/AP_HAL.h>
#include <string.h>
#include <stdio.h>  // for sprintf
#include "AP_PDRL_Commander_Logger.h"

AP_PDRL_Logger *AP_PDRL_Logger::m_AP_PDRL_Logger = 0;
extern const AP_HAL::HAL& hal;

AP_PDRL_Logger::AP_PDRL_Logger():
																										m_AP_NPNT_data_helper(AP_NPNT_data_helper::getInstance()),
																										_geofence_file_fd(-1)
{

}

AP_PDRL_Logger *AP_PDRL_Logger::getInstance()
{
	if(!m_AP_PDRL_Logger)
		m_AP_PDRL_Logger = new AP_PDRL_Logger();
	return m_AP_PDRL_Logger;
}

bool AP_PDRL_Logger::file_exists(const char *filename) const
{
	struct stat st;
	EXPECT_DELAY_MS(3000);
	if (AP::FS().stat(filename, &st) == -1) {
		// hopefully errno==ENOENT.  If some error occurs it is
		// probably better to assume this file exists.
		return false;
	}
	return true;
}

int64_t AP_PDRL_Logger::disk_space_avail()
{
#if CONFIG_HAL_BOARD != HAL_BOARD_SITL
	#if HAL_OS_POSIX_IO
		struct statfs _stats;
		if (statfs(_log_directory, &_stats) < 0) {
			return -1;
		}
		return (((int64_t)_stats.f_bavail) * _stats.f_bsize);
	#elif HAL_OS_FATFS_IO
		//	return fs_getfree();
		return AP::FS().disk_free("/");
	#else
		// return a fake disk space size
		return 100*1000*1000UL;
	#endif
#else
		return 100*1000*1000UL;
#endif
}

bool AP_PDRL_Logger::addCordinateToLog(char* entryType,double latitude,double longitude,float altc,uint64_t epoch)
{

	/*
	if (_geofence_file_fd > -1) {
		AP::FS().close(_geofence_file_fd);
		_geofence_file_fd = -1;
	}

	//	if (disk_space_avail() < _free_space_min_avail) {
	//		hal.console->printf("Out of space for logging\n");
	//		return -1;
	//	}

	if (!write_fd_semaphore.take(1)) {
		_open_error = true;
		return -1;
	}
	if (!write_filename) {
		return -1;
	}

	if (!file_exists(write_filename)) {
		_geofence_file_fd = AP::FS().open(write_filename,O_APPEND | O_CREAT | O_WRONLY);
	}
	else
	{
		_geofence_file_fd = AP::FS().open(write_filename, O_APPEND | O_WRONLY);
	}

	//	char dBuf[15];
	char lineBuf[100] = {0};
	snprintf(lineBuf,sizeof(lineBuf),"%s,%3.7lf,%3.7lf,%lf,%llu\n",entryType,(float)(latitude/10000000),(float)(longitude/10000000),altitude,(unsigned long long)epoch);

	ssize_t to_write = strlen(lineBuf);
	EXPECT_DELAY_MS(2000);
	const ssize_t written = AP::FS().write(_geofence_file_fd, lineBuf, to_write);
	//free(lineBuf);
	if (written < to_write) {
		_open_error = true;
		write_fd_semaphore.give();
		AP::FS().close(_geofence_file_fd);
		_geofence_file_fd = -1;
		return -1;
	}
	write_fd_semaphore.give();
	AP::FS().close(_geofence_file_fd);
	_geofence_file_fd = -1;
	return 0;
	 */

	int landOff_file_fd = AP::FS().open(write_filename,O_APPEND | O_CREAT | O_WRONLY);
	if(landOff_file_fd == -1)
	{
		AP::FS().close(landOff_file_fd);
		return -1;
	}
	char lineBuf[100];
	//float altc = (float)((double)altitude * (double)0.01);
	snprintf(lineBuf,sizeof(lineBuf),"%s,%3.7lf,%3.7lf,%.2f,%llu\n",entryType,(float)(latitude/10000000),(float)(longitude/10000000),altc,(unsigned long long)((epoch/1000000U)+(AP::rtc().tz_min*60U) ));
	AP::FS().write(landOff_file_fd,lineBuf,strlen(lineBuf));
	AP::FS().close(landOff_file_fd);
	return 0;
}

size_t AP_PDRL_Logger::fileSize(char* filename)
{
#if 0
	struct stat  file_info;

	int retVal = stat(filename,&file_info);
	if ( (filename != NULL) && (retVal == 0) )  //NULL check/stat() call
		return (size_t)file_info.st_size;  // Note: this may not fit in a size_t variable

	return 0;
#else
	struct stat  file_info;

	AP::FS().stat(filename,&file_info);
	//if ( (filename != NULL) && (retVal == 0) )  //NULL check/stat() call
	return (size_t)file_info.st_size;  // Note: this may not fit in a size_t variable

	return 0;
#endif
}

int AP_PDRL_Logger::startReadingLogFile()
{
	char currentFileName[42] = {0};
	size_t logFileSize = 0;

	memcpy(&currentFileName,&write_filename,36);
	strcat(currentFileName,".json");
	//asprintf(&currentFileName,"%s.json",(char*)write_filename);

	if (_current_log_file_fd > -1) {
		AP::FS().close(_current_log_file_fd);
		_current_log_file_fd = -1;
	}
	//check requested file is present
	if (!file_exists(currentFileName))
	{
		//		free(currentFileName);
		return -1;
	}
	else
	{
		logFileSize = fileSize(currentFileName);
		_current_log_file_fd = AP::FS().open(currentFileName, O_RDONLY);
		if(_current_log_file_fd == -1)
		{
			AP::FS().close(_current_log_file_fd);
			//			free(currentFileName);
			return -1;
		}
	}
	//	free(currentFileName);
	return logFileSize;
}

int AP_PDRL_Logger::getLogFileInBuffer(uint8_t* byBuffer,uint16_t readSize)
{
	EXPECT_DELAY_MS(100);
	int byRead = AP::FS().read(_current_log_file_fd,byBuffer,readSize);
	if(byRead > 0)
		return byRead;

	AP::FS().close(_current_log_file_fd);
	return 0;
}

int AP_PDRL_Logger::getLogFileOffsetInBuffer(uint8_t* byBuffer,uint16_t fileoffset, uint16_t readSize, uint8_t isSetSeek)
{
	EXPECT_DELAY_MS(100);
	if(isSetSeek)
		AP::FS().lseek(_current_log_file_fd,fileoffset, SEEK_SET);
	int byRead = AP::FS().read(_current_log_file_fd,byBuffer,readSize);
	if(byRead > 0)
		return byRead;

	return 0;
}

int AP_PDRL_Logger::sendLogFileToGCS(char* filename, int fileIndex)
{
	int logFileFd = -1;
	if (!write_fd_semaphore.take(1)) {
		_open_error = true;
		return -1;
	}

	//check requested file is present
	if (!file_exists(filename))
	{
		write_fd_semaphore.give();
		return -1;
	}
	else
	{
		logFileFd = AP::FS().open(filename, O_RDONLY);
		if(logFileFd == -1)
		{
			write_fd_semaphore.give();
			AP::FS().close(logFileFd);
			return -1;
		}
	}

	uint8_t byBuffer[100];
	int byCount = 0;
	while( (byCount = AP::FS().read(logFileFd,byBuffer,100)) > 0)
	{
		//send byCount bytes to mavlin
		memset(byBuffer,0,sizeof(byBuffer));
	}

	write_fd_semaphore.give();
	return 0;
}

int AP_PDRL_Logger::start_new_geofence_log(char* m_write_filename)
{
#if QCI_TEST_LOG
	currentDebugLogType = 0;
#endif
	memset(write_filename,0,sizeof(write_filename));
	if(m_write_filename != nullptr)
		strcpy(write_filename,m_write_filename);
	if (_geofence_file_fd > -1) {
		AP::FS().close(_geofence_file_fd);
		_geofence_file_fd = -1;
	}

	if (disk_space_avail() < _free_space_min_avail) {
		return -1;
	}

	if (!write_fd_semaphore.take(1)) {
		_open_error = true;
		return -1;
	}
	if (!write_filename) {
		return -1;
	}

	if (file_exists(write_filename)) {
		EXPECT_DELAY_MS(2000);
		//if (AP::FS().unlink(write_filename) == -1) //delete raw log file for current PA
		{
			if (errno == ENOENT) {

			}
		}
		char *currentFileName = nullptr;
		if(asprintf(&currentFileName,"%s.json",(char*)write_filename)){}	//delete json file for current PA
		//AP::FS().unlink(currentFileName);
		free(currentFileName);
	}
#if HAL_OS_POSIX_IO
	_geofence_file_fd = AP::FS().open(write_filename, O_RDWR | O_CREAT, 0666);
#else
	//TODO add support for mode flags
	_geofence_file_fd = AP::FS().open(write_filename, O_RDWR | O_CREAT);
#endif

	if (_geofence_file_fd == -1) {
		write_fd_semaphore.give();
		return -1;
	}
	AP::FS().close(_geofence_file_fd);

	write_fd_semaphore.give();
	return 0;
}

int AP_PDRL_Logger::makeJsonAndSignLogFile(char* outFilepath)

{
	int outPutFile_fd = -1;
	if (!write_fd_semaphore.take(1))
	{
		_open_error = true;
		return -1;
	}
	if (_geofence_file_fd > -1)
	{
		AP::FS().close(_geofence_file_fd);
		_geofence_file_fd = -1;
	}

	if (!outFilepath)
	{
		write_fd_semaphore.give();
		return -1;
	}

	if (file_exists(outFilepath))
	{
		AP::FS().unlink((char*)outFilepath);
	}

	//	_geofence_file_fd = AP::FS().open("GeoFenceBreach.txt", O_RDONLY);
	outPutFile_fd = AP::FS().open(outFilepath, O_WRONLY | O_CREAT | O_TRUNC);
	if(outPutFile_fd == -1)
	{
		write_fd_semaphore.give();
		return -1;
	}


	AP::FS().write(outPutFile_fd,"{\n\"PermissionArtefact\": \"",strlen("{\n\"PermissionArtefact\": \""));
	AP::FS().write(outPutFile_fd,m_AP_NPNT_data_helper->permissionArticaftID,strlen(m_AP_NPNT_data_helper->permissionArticaftID));

	AP::FS().write(outPutFile_fd,"\",\n\"FlightLog\": {\n\"GeofenceBreach\": [\n", strlen("\",\n\"FlightLog\": {\n\"GeofenceBreach\": [\n"));

	//	char chunk[128];
	char by[1];
	int count = 0;
	int currentLocation = 0;

	if (file_exists("GeoFenceBreach.txt"))
	{
		count = 0;
		currentLocation = 0;
		while(AP::FS().read(_geofence_file_fd,by, 1) > 0)
		{
			count++;
			if(by[0] == '\n')
			{
				char buf[100];
				memset(buf,0,sizeof(buf));
				AP::FS().lseek(_geofence_file_fd,currentLocation, SEEK_SET);
				AP::FS().read(_geofence_file_fd,buf,count);
				currentLocation += count;
				AP::FS().lseek(_geofence_file_fd,currentLocation, SEEK_SET);

				//process line here and make jeson
				EXPECT_DELAY_MS(2000);
				AP::FS().write(outPutFile_fd,"{",1);
				char *p;
				p = strtok (buf,",");
				//store lat
				AP::FS().write(outPutFile_fd,"\n\"Latitude\":",strlen("\n\"Latitude\":"));
				AP::FS().write(outPutFile_fd,p,strlen(p));

				//store long
				p = strtok (NULL, ",");
				AP::FS().write(outPutFile_fd,",\n\"Longitude\":",strlen(",\n\"Longitude\":"));
				AP::FS().write(outPutFile_fd,p,strlen(p));

				//store altitude
				p = strtok (NULL, ",");
				AP::FS().write(outPutFile_fd,",\n\"Altitude\":",strlen(",\n\"Altitude\":"));
				AP::FS().write(outPutFile_fd,p,strlen(p));

				//store epoch
				p = strtok (NULL, ",");
				AP::FS().write(outPutFile_fd,",\n\"TimeStamp\":",strlen(",\n\"TimeStamp\":"));
				AP::FS().write(outPutFile_fd,p,strlen(p));

				AP::FS().write(outPutFile_fd,"},\n",strlen("},\n"));
				//free(buf);
				count = 0;
			}
		}
		if(currentLocation)
		{
			AP::FS().lseek(outPutFile_fd,-2, SEEK_CUR);
		}
		AP::FS().close(_geofence_file_fd);
	}
	AP::FS().write(outPutFile_fd," ",strlen(" "));
	AP::FS().write(outPutFile_fd,"\n],\n",strlen("\n],\n"));


	AP::FS().write(outPutFile_fd,"\"TimeBreach\": [\n",strlen("\"TimeBreach\": [\n"));

	count = 0;
	currentLocation = 0;

	if (file_exists("TimeBreach.txt"))
	{
		count = 0;
		currentLocation = 0;
		_geofence_file_fd = AP::FS().open("TimeBreach.txt", O_RDONLY);
		while(AP::FS().read(_geofence_file_fd,by, 1) > 0)
		{
			count++;
			if(by[0] == '\n')
			{
				char buf[100];
				memset(buf,0,sizeof(buf));
				AP::FS().lseek(_geofence_file_fd,currentLocation, SEEK_SET);
				AP::FS().read(_geofence_file_fd,buf,count);
				currentLocation += count;
				AP::FS().lseek(_geofence_file_fd,currentLocation, SEEK_SET);

				//process line here and make jeson
				EXPECT_DELAY_MS(2000);
				AP::FS().write(outPutFile_fd,"{",1);
				char *p;
				p = strtok (buf,",");
				//store lat
				AP::FS().write(outPutFile_fd,"\n\"Latitude\":",strlen("\n\"Latitude\":"));
				AP::FS().write(outPutFile_fd,p,strlen(p));

				//store long
				p = strtok (NULL, ",");
				AP::FS().write(outPutFile_fd,",\n\"Longitude\":",strlen(",\n\"Longitude\":"));
				AP::FS().write(outPutFile_fd,p,strlen(p));

				//store altitude
				p = strtok (NULL, ",");
				AP::FS().write(outPutFile_fd,",\n\"Altitude\":",strlen(",\n\"Altitude\":"));
				AP::FS().write(outPutFile_fd,p,strlen(p));

				//store epoch
				p = strtok (NULL, ",");
				AP::FS().write(outPutFile_fd,",\n\"TimeStamp\":",strlen(",\n\"TimeStamp\":"));
				AP::FS().write(outPutFile_fd,p,strlen(p));

				AP::FS().write(outPutFile_fd,"},\n",strlen("},\n"));
				//free(buf);
				count = 0;
			}
		}
		if(currentLocation)
		{
			AP::FS().lseek(outPutFile_fd,-2, SEEK_CUR);
		}
		AP::FS().close(_geofence_file_fd);
	}
	AP::FS().write(outPutFile_fd," ",strlen(" "));
	AP::FS().write(outPutFile_fd,"\n],\n",strlen("\n],\n"));


	//-------------------add land log-----------------------------
	if (file_exists("Land.txt"))
	{
		count = 0;
		currentLocation = 0;
		_geofence_file_fd = AP::FS().open("Land.txt", O_RDONLY);
		AP::FS().write(outPutFile_fd,"\"Land\":",strlen("\"Land\":"));
		while(AP::FS().read(_geofence_file_fd,by, 1) > 0)
		{
			count++;
			if(by[0] == '\n')
			{
				char buf[100];
				memset(buf,0,sizeof(buf));
				AP::FS().lseek(_geofence_file_fd,currentLocation, SEEK_SET);
				AP::FS().read(_geofence_file_fd,buf,count);
				currentLocation += count;
				AP::FS().lseek(_geofence_file_fd,currentLocation, SEEK_SET);

				//process line here and make jeson
				EXPECT_DELAY_MS(2000);
				AP::FS().write(outPutFile_fd,"{",1);
				char *p;
				p = strtok (buf,",");
				//store lat
				AP::FS().write(outPutFile_fd,"\n\"Latitude\":",strlen("\n\"Latitude\":"));
				AP::FS().write(outPutFile_fd,p,strlen(p));

				//store long
				p = strtok (NULL, ",");
				AP::FS().write(outPutFile_fd,",\n\"Longitude\":",strlen(",\n\"Longitude\":"));
				AP::FS().write(outPutFile_fd,p,strlen(p));

				//store altitude
				p = strtok (NULL, ",");
				AP::FS().write(outPutFile_fd,",\n\"Altitude\":",strlen(",\n\"Altitude\":"));
				AP::FS().write(outPutFile_fd,p,strlen(p));

				//store epoch
				p = strtok (NULL, ",");
				AP::FS().write(outPutFile_fd,",\n\"TimeStamp\":",strlen(",\n\"TimeStamp\":"));
				AP::FS().write(outPutFile_fd,p,strlen(p));

				AP::FS().write(outPutFile_fd,"},\n",strlen("},\n"));
				//free(buf);
				count = 0;
				break;
			}
		}
		AP::FS().close(_geofence_file_fd);
	}
	else
	{
		char lineBuf[60];

		AP::FS().write(outPutFile_fd,"\"Land\":{\n",strlen("\"Land\":{\n"));

		AP::FS().write(outPutFile_fd,"\"Latitude\":",strlen("\"Latitude\":"));

		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)(LandOffLocTime.lat/10000000));

		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));

		AP::FS().write(outPutFile_fd,",\n\"Longitude\":",strlen(",\n\"Longitude\":"));

		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)(LandOffLocTime.lng/10000000));

		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));

		AP::FS().write(outPutFile_fd,",\n\"Altitude\": 0 ",strlen(",\n\"Altitude\": 0 "));

		AP::FS().write(outPutFile_fd,",\n\"TimeStamp\":",strlen(",\n\"TimeStamp\":"));

		snprintf(lineBuf,sizeof(lineBuf),"%u",(unsigned int)LandOffLocTime.time);

		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));

		AP::FS().write(outPutFile_fd,"\n},\n",strlen("\n},\n"));
	}

	//-------------------add take off log-----------------------------
	if (file_exists("TakeOff.txt"))
	{
		count = 0;
		currentLocation = 0;
		_geofence_file_fd = AP::FS().open("TakeOff.txt", O_RDONLY);
		AP::FS().write(outPutFile_fd,"\"TakeOff\":",strlen("\"TakeOff\":"));
		while(AP::FS().read(_geofence_file_fd,by, 1) > 0)
		{
			count++;
			if(by[0] == '\n')
			{
				char buf[100];
				memset(buf,0,sizeof(buf));
				AP::FS().lseek(_geofence_file_fd,currentLocation, SEEK_SET);
				AP::FS().read(_geofence_file_fd,buf,count);
				currentLocation += count;
				AP::FS().lseek(_geofence_file_fd,currentLocation, SEEK_SET);

				//process line here and make jeson
				EXPECT_DELAY_MS(2000);
				AP::FS().write(outPutFile_fd,"{",1);
				char *p;
				p = strtok (buf,",");
				//store lat
				AP::FS().write(outPutFile_fd,"\n\"Latitude\":",strlen("\n\"Latitude\":"));
				AP::FS().write(outPutFile_fd,p,strlen(p));

				//store long
				p = strtok (NULL, ",");
				AP::FS().write(outPutFile_fd,",\n\"Longitude\":",strlen(",\n\"Longitude\":"));
				AP::FS().write(outPutFile_fd,p,strlen(p));

				//store altitude
				p = strtok (NULL, ",");
				AP::FS().write(outPutFile_fd,",\n\"Altitude\":",strlen(",\n\"Altitude\":"));
				AP::FS().write(outPutFile_fd,p,strlen(p));

				//store epoch
				p = strtok (NULL, ",");
				AP::FS().write(outPutFile_fd,",\n\"TimeStamp\":",strlen(",\n\"TimeStamp\":"));
				AP::FS().write(outPutFile_fd,p,strlen(p));

				AP::FS().write(outPutFile_fd,"\n}\n}\n}\n\0",strlen("\n}\n}\n}\n\0"));
				//free(buf);
				count = 0;
				break;
			}
		}
		AP::FS().close(_geofence_file_fd);
	}
	else
	{
		char lineBuf[60];


		AP::FS().write(outPutFile_fd,"\"TakeOff\":{\n",strlen("\"TakeOff\":{\n"));

		AP::FS().write(outPutFile_fd,"\"Latitude\":",strlen("\"Latitude\":"));

		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)(takeOffLocTime.lat/10000000));

		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));

		AP::FS().write(outPutFile_fd,",\n\"Longitude\":",strlen(",\n\"Longitude\":"));

		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)(takeOffLocTime.lng/10000000));

		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));

		AP::FS().write(outPutFile_fd,",\n\"Altitude\": 0 ",strlen(",\n\"Altitude\": 0 "));

		AP::FS().write(outPutFile_fd,",\n\"TimeStamp\":",strlen(",\n\"TimeStamp\":"));

		snprintf(lineBuf,sizeof(lineBuf),"%u",(unsigned int)takeOffLocTime.time);

		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));

		AP::FS().write(outPutFile_fd,"\n}\n}\n}\n\0",strlen("\n}\n}\n}\n\0"));
	}


	//	AP::FS().write(outPutFile_fd,"},\n\"Signature\": \"dM8P/gSoXtu8po6/AyCLXyGQkDZMLwLE5IXDI7EJQ6IJY1Ih3VRxrtIQ5ReAQb0T0lSf8eQYPfPt1vdSBK3zukL7lhz5ZiXmmYUFsdQYKclz/in1eNhkGgDSgpjqmQEJMcrFzw9r4aQUozP++6a3QaiRAGnFeglsqJ9vuXlINclL+TOyVx8wZGkAxrZPMe3RTemde3a9+Tq0LGzbX28770O2LclHTSGYpFULmydZuOD5pX2PmzTNsSrgL68c3RgVMWWKrvzdtDzVdtFuQ4DowkbOqdq8UIuK4k6YuHbI2rbJlCF1ERwMWZQTHyRUkf0e6hhhhH8SdUaWXIMzuxw6xQ==\"\n}",strlen("},\n\"Signature\": \"dM8P/gSoXtu8po6/AyCLXyGQkDZMLwLE5IXDI7EJQ6IJY1Ih3VRxrtIQ5ReAQb0T0lSf8eQYPfPt1vdSBK3zukL7lhz5ZiXmmYUFsdQYKclz/in1eNhkGgDSgpjqmQEJMcrFzw9r4aQUozP++6a3QaiRAGnFeglsqJ9vuXlINclL+TOyVx8wZGkAxrZPMe3RTemde3a9+Tq0LGzbX28770O2LclHTSGYpFULmydZuOD5pX2PmzTNsSrgL68c3RgVMWWKrvzdtDzVdtFuQ4DowkbOqdq8UIuK4k6YuHbI2rbJlCF1ERwMWZQTHyRUkf0e6hhhhH8SdUaWXIMzuxw6xQ==\"\n}"));

	AP::FS().close(_geofence_file_fd);
	AP::FS().close(outPutFile_fd);
	write_fd_semaphore.give();
	//signLogFile(outFilepath);
	return 0;
}

#if QCI_TEST_LOG
void AP_PDRL_Logger::qciLogGenerationTest(int outPutFile_fd)
{
	if(currentDebugLogType == 0)
	{
		EXPECT_DELAY_MS(1000);
		char lineBuf[60] = {0};
		AP::FS().write(outPutFile_fd,"{\"entryType\":\"TAKEOFF/ARM\",",strlen("{\"entryType\":\"TAKEOFF/ARM\","));
		AP::FS().write(outPutFile_fd,"\"latitude\":",strlen("\"latitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)20.0028011f);
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"longitude\":",strlen(",\"longitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)73.7214145f);
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"altitude\":",strlen(",\"altitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)(0));
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"timeStamp\":",strlen(",\"timeStamp\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%u",(unsigned int)((m_AP_NPNT_data_helper->flightStartTime)+1));
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,"},",strlen("},"));
		AP::FS().write(outPutFile_fd,"{\"entryType\":\"LAND/DISARM\",",strlen("{\"entryType\":\"LAND/DISARM\","));
		AP::FS().write(outPutFile_fd,"\"latitude\":",strlen("\"latitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)20.0028011f);
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"longitude\":",strlen(",\"longitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)73.7214145f);
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"altitude\":",strlen(",\"altitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)(1));
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"timeStamp\":",strlen(",\"timeStamp\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%u",(unsigned int)((m_AP_NPNT_data_helper->flightStartTime)+2));
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
	}
	else if(currentDebugLogType == 1)
	{
		EXPECT_DELAY_MS(1000);
		char lineBuf[60] = {0};
		AP::FS().write(outPutFile_fd,"{\"entryType\":\"TAKEOFF/ARM\",",strlen("{\"entryType\":\"TAKEOFF/ARM\","));
		AP::FS().write(outPutFile_fd,"\"latitude\":",strlen("\"latitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)20.0028011f);
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"longitude\":",strlen(",\"longitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)73.7214145f);
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"altitude\":",strlen(",\"altitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)(0));
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"timeStamp\":",strlen(",\"timeStamp\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%u",(unsigned int)((m_AP_NPNT_data_helper->flightStartTime)+3));
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,"},",strlen("},"));
		AP::FS().write(outPutFile_fd,"{\"entryType\":\"LAND/DISARM\",",strlen("{\"entryType\":\"LAND/DISARM\","));
		AP::FS().write(outPutFile_fd,"\"latitude\":",strlen("\"latitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)20.0028011f);
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"longitude\":",strlen(",\"longitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)73.7214145f);
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"altitude\":",strlen(",\"altitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)(1));
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"timeStamp\":",strlen(",\"timeStamp\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%u",(unsigned int)((m_AP_NPNT_data_helper->flightStartTime)+4));
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
	}
	else if(currentDebugLogType == 2)
	{
		EXPECT_DELAY_MS(1000);
		char lineBuf[60] = {0};
		AP::FS().write(outPutFile_fd,"{\"entryType\":\"TAKEOFF/ARM\",",strlen("{\"entryType\":\"TAKEOFF/ARM\","));
		AP::FS().write(outPutFile_fd,"\"latitude\":",strlen("\"latitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)20.0028011f);
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"longitude\":",strlen(",\"longitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)73.7214145f);
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"altitude\":",strlen(",\"altitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)(0));
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"timeStamp\":",strlen(",\"timeStamp\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%u",(unsigned int)((m_AP_NPNT_data_helper->flightStartTime)+5));
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,"},",strlen("},"));
		AP::FS().write(outPutFile_fd,"{\"entryType\":\"GEOFENCE_BREACH\",",strlen("{\"entryType\":\"GEOFENCE_BREACH\","));
		AP::FS().write(outPutFile_fd,"\"latitude\":",strlen("\"latitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)19.9886663f);
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"longitude\":",strlen(",\"longitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)73.7115364f);
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"altitude\":",strlen(",\"altitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)(5));
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"timeStamp\":",strlen(",\"timeStamp\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%u",(unsigned int)((m_AP_NPNT_data_helper->flightStartTime)+6));
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,"},",strlen("},"));
		AP::FS().write(outPutFile_fd,"{\"entryType\":\"GEOFENCE_BREACH\",",strlen("{\"entryType\":\"GEOFENCE_BREACH\","));
		AP::FS().write(outPutFile_fd,"\"latitude\":",strlen("\"latitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)19.9886663f);
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"longitude\":",strlen(",\"longitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)73.7115364f);
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"altitude\":",strlen(",\"altitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)(5));
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"timeStamp\":",strlen(",\"timeStamp\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%u",(unsigned int)((m_AP_NPNT_data_helper->flightStartTime)+7));
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,"},",strlen("},"));
		AP::FS().write(outPutFile_fd,"{\"entryType\":\"LAND/DISARM\",",strlen("{\"entryType\":\"LAND/DISARM\","));
		AP::FS().write(outPutFile_fd,"\"latitude\":",strlen("\"latitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)20.0028011f);
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"longitude\":",strlen(",\"longitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)73.7214145f);
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"altitude\":",strlen(",\"altitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)(1));
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"timeStamp\":",strlen(",\"timeStamp\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%u",(unsigned int)((m_AP_NPNT_data_helper->flightStartTime)+8));
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
	}
	else if(currentDebugLogType == 3)
	{
		EXPECT_DELAY_MS(1000);
		char lineBuf[60] = {0};
		AP::FS().write(outPutFile_fd,"{\"entryType\":\"TAKEOFF/ARM\",",strlen("{\"entryType\":\"TAKEOFF/ARM\","));
		AP::FS().write(outPutFile_fd,"\"latitude\":",strlen("\"latitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)20.0028011f);
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"longitude\":",strlen(",\"longitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)73.7214145f);
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"altitude\":",strlen(",\"altitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)(0));
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"timeStamp\":",strlen(",\"timeStamp\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%u",(unsigned int)((m_AP_NPNT_data_helper->flightStartTime)+9));
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,"},",strlen("},"));
		AP::FS().write(outPutFile_fd,"{\"entryType\":\"TIME_BREACH\",",strlen("{\"entryType\":\"TIME_BREACH\","));
		AP::FS().write(outPutFile_fd,"\"latitude\":",strlen("\"latitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)20.0028011f);
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"longitude\":",strlen(",\"longitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)73.7214145f);
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"altitude\":",strlen(",\"altitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)(5));
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"timeStamp\":",strlen(",\"timeStamp\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%u",(unsigned int)((m_AP_NPNT_data_helper->flightEndTime)+1));
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,"},",strlen("},"));
		AP::FS().write(outPutFile_fd,"{\"entryType\":\"TIME_BREACH\",",strlen("{\"entryType\":\"TIME_BREACH\","));
		AP::FS().write(outPutFile_fd,"\"latitude\":",strlen("\"latitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)20.0028011f);
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"longitude\":",strlen(",\"longitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)73.7214145f);
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"altitude\":",strlen(",\"altitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)(5));
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"timeStamp\":",strlen(",\"timeStamp\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%u",(unsigned int)((m_AP_NPNT_data_helper->flightEndTime)+2));
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,"},",strlen("},"));
		AP::FS().write(outPutFile_fd,"{\"entryType\":\"LAND/DISARM\",",strlen("{\"entryType\":\"LAND/DISARM\","));
		AP::FS().write(outPutFile_fd,"\"latitude\":",strlen("\"latitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)20.0028011f);
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"longitude\":",strlen(",\"longitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)73.7214145f);
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"altitude\":",strlen(",\"altitude\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)(1));
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		AP::FS().write(outPutFile_fd,",\"timeStamp\":",strlen(",\"timeStamp\":"));
		memset(lineBuf,0,sizeof(lineBuf));
		snprintf(lineBuf,sizeof(lineBuf),"%u",(unsigned int)((m_AP_NPNT_data_helper->flightEndTime)+3));
		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));
		currentDebugLogType = -1;
	}
	AP::FS().write(outPutFile_fd,"}]}}\0",strlen("}]}}\0"));
	currentDebugLogType++;
}
#endif

int AP_PDRL_Logger::makeJsonAndSignLogFileLatest()
{
	int outPutFile_fd = -1;
	uint8_t isOldHashAvailable = 0;
	char currentFileName[42] = {0};
	char currentHashFileName[42] = {0};
	if (!write_fd_semaphore.take(1))
	{
		_open_error = true;
		return -1;
	}
	if (_geofence_file_fd > -1)
	{
		AP::FS().close(_geofence_file_fd);
		_geofence_file_fd = -1;
	}

	_geofence_file_fd = AP::FS().open(write_filename, O_RDONLY);

	memcpy(&currentHashFileName,&write_filename,36);
	strcat(currentHashFileName,".hash");
	if (file_exists(currentHashFileName))
	{
		memset(&oldLogFileHash,0,sizeof(oldLogFileHash));
		int hashFile_fd = AP::FS().open(currentHashFileName, O_RDONLY);
		if(32 == AP::FS().read(hashFile_fd ,oldLogFileHash, 32))
		{
			isOldHashAvailable = 1;
		}
		AP::FS().close(hashFile_fd);
	}

	memcpy(&currentFileName,&write_filename,36);
	strcat(currentFileName,".json");
	if (file_exists(currentFileName))
	{
		AP::FS().unlink((char*)currentFileName);
	}
	//	_geofence_file_fd = AP::FS().open("GeoFenceBreach.txt", O_RDONLY);
	outPutFile_fd = AP::FS().open(currentFileName, O_WRONLY | O_CREAT);
	if(outPutFile_fd == -1)
	{
		write_fd_semaphore.give();
		//		free(currentFileName);
		return -1;
	}

	EXPECT_DELAY_MS(1000);

	AP::FS().write(outPutFile_fd,"{\"flightLog\":{\"permissionArtefact\":\"",strlen("{\"flightLog\":{\"permissionArtefact\":\""));
	//	AP::FS().write(outPutFile_fd,m_AP_NPNT_data_helper->permissionArticaftID,strlen(m_AP_NPNT_data_helper->permissionArticaftID));
	AP::FS().write(outPutFile_fd,write_filename,36);//strlen(write_filename));

	AP::FS().write(outPutFile_fd,"\",\"previousLogHash\":\"",strlen("\",\"previousLogHash\":\""));

	if(isOldHashAvailable)
	{
		char *hexVal = nullptr;

		for(int i = 0; i < 32 ; i++)
		{
			if(asprintf(&hexVal,"%02x",oldLogFileHash[i]) > 0)
			AP::FS().write(outPutFile_fd,hexVal,2);
			free(hexVal);
		}
	}

	AP::FS().write(outPutFile_fd,"\",\"logEntries\":[", strlen("\",\"logEntries\":["));


#if QCI_TEST_LOG
	qciLogGenerationTest(outPutFile_fd);
#else
	char by[6] = {0};
	int count = 0;
	int currentLocation = 0;
	bool isRawLogFilePresent = false;
	//-------------------add take off log-----------------------------
	if (file_exists(write_filename) && (_geofence_file_fd > -1))
	{
		count = 0;
		currentLocation = 0;

		while(AP::FS().read(_geofence_file_fd,by, 1) > 0)
		{
			count++;
			if(by[0] == '\n')
			{
				EXPECT_DELAY_MS(1000);
				char buf[100];
				memset(buf,0,sizeof(buf));
				//go back to start of line
				AP::FS().lseek(_geofence_file_fd,currentLocation, SEEK_SET);
				AP::FS().read(_geofence_file_fd,buf,count);
				currentLocation += count;
				AP::FS().lseek(_geofence_file_fd,currentLocation, SEEK_SET);

				//process line here and make jeson
				EXPECT_DELAY_MS(2000);

				char *p;
				p = strtok (buf,",");
				if(p == nullptr)
				{
					by[0] = 0;
					continue;
				}

				isRawLogFilePresent = true;

				AP::FS().write(outPutFile_fd,"{\"entryType\":\"",strlen("{\"entryType\":\""));

				AP::FS().write(outPutFile_fd,p,strlen(p));

				//store lat
				p = strtok (NULL,",");
				AP::FS().write(outPutFile_fd,"\",\"latitude\":",strlen("\",\"latitude\":"));
				AP::FS().write(outPutFile_fd,p,strlen(p));

				//store long
				p = strtok (NULL, ",");
				AP::FS().write(outPutFile_fd,",\"longitude\":",strlen(",\"longitude\":"));
				AP::FS().write(outPutFile_fd,p,strlen(p));

				//store altitude
				p = strtok (NULL, ",");
				AP::FS().write(outPutFile_fd,",\"altitude\":",strlen(",\"altitude\":"));
				AP::FS().write(outPutFile_fd,p,strlen(p));

				//store epoch
				p = strtok (NULL, ",");
				AP::FS().write(outPutFile_fd,",\"timeStamp\":",strlen(",\"timeStamp\":"));
				AP::FS().write(outPutFile_fd,p,strlen(p));

				AP::FS().write(outPutFile_fd,"},",strlen("},"));
				//free(buf);
				count = 0;
			}
		}
	}
	//else
	if (isRawLogFilePresent == true) //means no line is process add default takeoff land location
	{
		AP::FS().close(_geofence_file_fd);
		AP::FS().lseek(outPutFile_fd,-1, SEEK_CUR);
		AP::FS().write(outPutFile_fd,"]}}\0",strlen("]}}\0"));
	}
	else
	{
		AP_AHRS &ahrs = AP::ahrs();
		const AP_GPS &gps = AP::gps();
		struct Location loc;
		if (!ahrs.get_location(loc)) {
		        return false;
		    }
		if (!ahrs.get_hagl(takeOffLocTime.alt)) {
		        return false;
		    }

		LandOffLocTime.lat = takeOffLocTime.lat = loc.lat;
		LandOffLocTime.lng = takeOffLocTime.lng = loc.lng;
		LandOffLocTime.time = takeOffLocTime.time = (gps.time_epoch_usec()/1000000U)+(AP::rtc().tz_min*60U);
		LandOffLocTime.alt = takeOffLocTime.alt;

		EXPECT_DELAY_MS(1000);
		char lineBuf[60] = {0};

		AP::FS().write(outPutFile_fd,"{\"entryType\":\"TAKEOFF/ARM\",",strlen("{\"entryType\":\"TAKEOFF/ARM\","));

		AP::FS().write(outPutFile_fd,"\"latitude\":",strlen("\"latitude\":"));

		memset(lineBuf,0,sizeof(lineBuf));

		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)(takeOffLocTime.lat/10000000));

		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));

		AP::FS().write(outPutFile_fd,",\"longitude\":",strlen(",\"longitude\":"));

		memset(lineBuf,0,sizeof(lineBuf));

		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)(takeOffLocTime.lng/10000000));

		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));

		AP::FS().write(outPutFile_fd,",\"altitude\":",strlen(",\"altitude\":"));

		memset(lineBuf,0,sizeof(lineBuf));

		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)(takeOffLocTime.alt));

		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));

		AP::FS().write(outPutFile_fd,",\"timeStamp\":",strlen(",\"timeStamp\":"));

		memset(lineBuf,0,sizeof(lineBuf));

		snprintf(lineBuf,sizeof(lineBuf),"%u",(unsigned int)(takeOffLocTime.time));

		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));

		AP::FS().write(outPutFile_fd,"},",strlen("},"));

		AP::FS().write(outPutFile_fd,"{\"entryType\":\"LAND/DISARM\",",strlen("{\"entryType\":\"LAND/DISARM\","));

		AP::FS().write(outPutFile_fd,"\"latitude\":",strlen("\"latitude\":"));

		memset(lineBuf,0,sizeof(lineBuf));

		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)(takeOffLocTime.lat/10000000));

		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));

		AP::FS().write(outPutFile_fd,",\"longitude\":",strlen(",\"longitude\":"));

		memset(lineBuf,0,sizeof(lineBuf));

		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)(takeOffLocTime.lng/10000000));

		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));

		AP::FS().write(outPutFile_fd,",\"altitude\":",strlen(",\"altitude\":"));

		memset(lineBuf,0,sizeof(lineBuf));

		snprintf(lineBuf,sizeof(lineBuf),"%3.7lf",(float)(LandOffLocTime.alt));

		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));

		AP::FS().write(outPutFile_fd,",\"timeStamp\":",strlen(",\"timeStamp\":"));

		memset(lineBuf,0,sizeof(lineBuf));

		snprintf(lineBuf,sizeof(lineBuf),"%u",(unsigned int)(takeOffLocTime.time));

		AP::FS().write(outPutFile_fd,lineBuf,strlen(lineBuf));

		AP::FS().write(outPutFile_fd,"}]}}\0",strlen("}]}}\0"));
	}
#endif

	AP::FS().close(outPutFile_fd);
	outPutFile_fd = -1;
#if 0//HASH_BASE64
	size_t fileDataSize = fileSize(currentFileName);
	size_t outPutLen = 0;
	outPutFile_fd = AP::FS().open(currentFileName, O_RDONLY);
	unsigned char hash[32] = {0};
	char tmpBuff[6];
	volatile int counter = 0;
	int readBy = AP::FS().read(outPutFile_fd,tmpBuff, 3);
	counter += readBy;
	mbedtls_base64_getHash256(tmpBuff,fileDataSize,&outPutLen,1,counter,(unsigned char*)&hash);
	while(readBy > 0)
	{
		readBy = AP::FS().read(outPutFile_fd,tmpBuff, 3);
		counter += readBy;
		if(readBy)
			mbedtls_base64_getHash256(tmpBuff,fileDataSize,&outPutLen,0,counter,(unsigned char*)&hash);
	}
#else
	outPutFile_fd = AP::FS().open(currentFileName, O_RDWR);
	static mbedtls_sha256_context sha;
	char tmpBuff[200] = {0};
	int readBy;

	mbedtls_sha256_init(&sha);
	mbedtls_sha256_starts(&sha, 0);

	EXPECT_DELAY_MS(5000);
	while((readBy = AP::FS().read(outPutFile_fd,tmpBuff, 200)) > 0)
	{
		mbedtls_sha256_update(&sha,(unsigned char*)tmpBuff,readBy);
		memset(tmpBuff,0,sizeof(tmpBuff));
	}

	mbedtls_sha256_finish(&sha,oldLogFileHash);

	hashBuffer = (char*)oldLogFileHash;

	if (file_exists(currentHashFileName))
	{
		AP::FS().unlink(currentHashFileName);
	}

	int hashFile_fd = AP::FS().open(currentHashFileName, O_WRONLY | O_CREAT);
	AP::FS().write(hashFile_fd,oldLogFileHash,32);
	AP::FS().close(hashFile_fd);


	unsigned char dataBuffer[360] = {0};
	size_t outPutLen = 0;
	size_t outLen = encryptHash((char*)&dataBuffer,sizeof(dataBuffer));
	unsigned char signature[360] = {0};
	//signature = mbedtls_base64_encode_dynamic(&outPutLen,dataBuffer,outLen);
	mbedtls_base64_encode((unsigned char*)&signature,360,&outPutLen,dataBuffer,outLen);

	EXPECT_DELAY_MS(5000);
	AP::FS().lseek(outPutFile_fd,-2, SEEK_END);
	AP::FS().write(outPutFile_fd,",\"signature\":\"",strlen(",\"signature\":\""));
	AP::FS().write(outPutFile_fd,signature,outPutLen);
	AP::FS().write(outPutFile_fd,"\"}}",strlen("\"}}"));
	EXPECT_DELAY_MS(10);

#endif
	AP::FS().close(_geofence_file_fd);
	AP::FS().close(outPutFile_fd);
	write_fd_semaphore.give();
	//signLogFile(outFilepath);
	return 0;
}

size_t AP_PDRL_Logger::getFileSignature(uint8_t *currentHashFileName,unsigned char* signatureBuff,size_t sigBuffSize)
{
//    uint16_t* fIdPtr = ( uint16_t* )currentHashFileName;
    char *filename = _log_file_name(currentHashFileName[1]<<8 | currentHashFileName[0]);
    if (filename == nullptr) {
        return false; // ?!
    }
    int  outPutFile_fd = AP::FS().open(filename, O_RDWR);
    if(outPutFile_fd == -1)
    {
        free(filename);
        return -1;
    }
    static mbedtls_sha256_context sha;
    char tmpBuff[1024] = {0};
    unsigned char tmpHashBuffer[32];
    int readBy;

    mbedtls_sha256_init(&sha);
    mbedtls_sha256_starts(&sha, 0);

    while((readBy = AP::FS().read(outPutFile_fd,tmpBuff, sizeof(tmpBuff))) > 0)
    {
        EXPECT_DELAY_MS(100);
        mbedtls_sha256_update(&sha,(unsigned char*)tmpBuff,readBy);
    }
    AP::FS().close(outPutFile_fd);

    mbedtls_sha256_finish(&sha,tmpHashBuffer);

    unsigned char dataBuffer[256] = {0};
    size_t outPutLen = 0;
    size_t outLen = encryptHashUsingPublicKey(tmpHashBuffer,dataBuffer,sizeof(dataBuffer));

    mbedtls_base64_encode(signatureBuff,sigBuffSize,&outPutLen,dataBuffer,outLen);
//    AP::FS().write(outPutFile_fd,signature,outPutLen);
    free(filename);
    return outPutLen;
}


int AP_PDRL_Logger::checkTimeBreach(uint64_t *currentTime)
{
	*currentTime = (*currentTime) + 19800000000;
	if(*currentTime < m_AP_NPNT_data_helper->flightStartTime)
	{
		//time breach
		return 1;
	}
	else if(*currentTime > m_AP_NPNT_data_helper->flightEndTime)
	{
		//time breach
		return 1;
	}
	return 0;
}



int AP_PDRL_Logger::setTakeOffLolation(AGPILOT_LANDED_STATE landState)
{
	if(takeOffLogged == 0 )
	{
		//check if take off detected make takeOffLogged = 1; and landDetected = 0
		if((landState == AGPILOT_LANDED_STATE_TAKEOFF)|| (landState == AGPILOT_LANDED_STATE_IN_AIR))
		{
			AP_AHRS &ahrs = AP::ahrs();
			const AP_GPS &gps = AP::gps();
			struct Location loc;
			if (!ahrs.get_location(loc)) {
			        return false;
			    }
			if (!ahrs.get_hagl(takeOffLocTime.alt)) {
			        return false;
			    }
			takeOffLocTime.lat = loc.lat;
			takeOffLocTime.lng = loc.lng;
			takeOffLocTime.time = gps.time_epoch_usec();
			takeOffLogged = 1;
			landDetected = 0;
			int takeOff_file_fd = AP::FS().open(write_filename,O_APPEND | O_CREAT | O_WRONLY);
			if(takeOff_file_fd == -1)
			{
				AP::FS().close(takeOff_file_fd);
				return -1;
			}
			char lineBuf[100] = {0};
			snprintf(lineBuf,sizeof(lineBuf),"TAKEOFF/ARM,%3.7lf,%3.7lf,%.2f,%llu\n",(float)((double)loc.lat/10000000),(float)((double)loc.lng/10000000),takeOffLocTime.alt ,(unsigned long long)((takeOffLocTime.time/1000000U)+(AP::rtc().tz_min*60U)));
			AP::FS().write(takeOff_file_fd,lineBuf,strlen(lineBuf));
			AP::FS().close(takeOff_file_fd);
		}
	}
	return 0;
}

int AP_PDRL_Logger::setLandLolation(AGPILOT_LANDED_STATE landState)
{
	if(landDetected == 0)
	{
		//check if land detected make landDetected = 1; takeOffLogged = 0
		if((landState == AGPILOT_LANDED_STATE_LANDING) || (landState == AGPILOT_LANDED_STATE_ON_GROUND))
		{
			AP_AHRS &ahrs = AP::ahrs();
			const AP_GPS &gps = AP::gps();
			struct Location loc;
			if (!ahrs.get_location(loc)) {
			        return false;
			    }
			if (!ahrs.get_hagl(LandOffLocTime.alt)) {
			        return false;
			    }

			LandOffLocTime.lat = loc.lat;
			LandOffLocTime.lng = loc.lng;
			LandOffLocTime.time = gps.time_epoch_usec();
			//LandOffLocTime.alt = (float)((double)loc.alt * (double)0.01);
			landDetected = 1;
			takeOffLogged = 0;
			int landOff_file_fd = AP::FS().open(write_filename,O_APPEND | O_CREAT | O_WRONLY);
			if(landOff_file_fd == -1)
			{
				AP::FS().close(landOff_file_fd);
				return -1;
			}
			char lineBuf[100] = {0};
			snprintf(lineBuf,sizeof(lineBuf),"LAND/DISARM,%3.7lf,%3.7lf,%.2f,%llu\n",(float)((double)loc.lat/10000000),(float)((double)loc.lng/10000000),LandOffLocTime.alt ,(unsigned long long)((LandOffLocTime.time/1000000U)+(AP::rtc().tz_min*60U)));
			AP::FS().write(landOff_file_fd,lineBuf,strlen(lineBuf));
			AP::FS().close(landOff_file_fd);
		}
	}
	return 0;
}

void AP_PDRL_Logger::logTakeOffLand(AGPILOT_LANDED_STATE landState)
{
	setTakeOffLolation(landState);
	setLandLolation(landState);
}

int AP_PDRL_Logger::mbedtls_md_file(const mbedtls_md_info_t *md_info, const char *path, unsigned char *output )
{
	int ret;
	size_t n;
	mbedtls_md_context_t ctx;
	unsigned char buf[1024];

	if( md_info == NULL )
		return( MBEDTLS_ERR_MD_BAD_INPUT_DATA );

	int fileFd = AP::FS().open(path, O_RDONLY);

	if( fileFd == -1)
		return( MBEDTLS_ERR_MD_FILE_IO_ERROR );

	mbedtls_md_init( &ctx );

	if( ( ret = mbedtls_md_setup(&ctx, md_info, 0 ) ) != 0 )
		goto cleanup;

	//    if( ( ret = md_info->starts_func((mbedtls_md_info_t*)ctx.md_ctx ) ) != 0 )
	//        goto cleanup;
	if((ret = mbedtls_md_starts(&ctx)) != 0)
		goto cleanup;

	while( ( n = AP::FS().read(fileFd,buf,sizeof( buf )) ) > 0 )
		if( ( ret = mbedtls_md_update(&ctx,buf,n) ) != 0 )
			goto cleanup;

		else
			ret = mbedtls_md_finish(&ctx, output);

	cleanup:
	AP::FS().close(fileFd);
	mbedtls_md_free( &ctx );
	return( ret );
}

void AP_PDRL_Logger::setHashBuffer(char* hash,uint64_t hashLen,uint64_t totalLen,uint64_t offset)
{
	if(offset == 0)
	{
		if(hashBuffer)
			free(hashBuffer);
		hashBuffer = (char*)malloc(sizeof(char)*totalLen);
	}
	memcpy(hashBuffer+offset,hash,hashLen);
}

void AP_PDRL_Logger::freeHashBuffer()
{
	free(hashBuffer);
}

size_t AP_PDRL_Logger::encryptHash(char* buffer,size_t dataLenght)
{
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_entropy_context entropy;
    int ret = 0;
    size_t olen = 0;
    const char *pers = "rsa_encrypt";
    //  AP_KEYSTORE *keyStore = AP_KEYSTORE::getInstance();
    keyStore = AP_KEYSTORE::getInstance();

    mbedtls_ctr_drbg_init( &ctr_drbg );
    mbedtls_entropy_init( &entropy );

    if( ( ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy,(const unsigned char *) pers,
            strlen( pers ) ) ) != 0 )
    {
        return olen;
    }

	mbedtls_pk_context pk;
	mbedtls_pk_init(&pk);
	ret = mbedtls_pk_parse_public_key( &pk,(unsigned char*)pdrlPublicKey ,strlen(pdrlPublicKey)+1);
	if( ( ret = mbedtls_pk_encrypt( &pk,
			(const unsigned char *)hashBuffer,
			32,
			(unsigned char *)buffer,
			&olen, dataLenght,
			mbedtls_ctr_drbg_random,
			&ctr_drbg ) ) != 0 )
	{
		goto exit;
	}

    exit:
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );
    return olen;
}

size_t AP_PDRL_Logger::encryptHashUsingPublicKey(unsigned char* hashBuff,unsigned char* outBuff,size_t outBuffSize)
{
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_entropy_context entropy;
    int ret = 0;
    size_t olen = 0;
    const char *pers = "rsa_encrypt";

    mbedtls_ctr_drbg_init( &ctr_drbg );
    mbedtls_entropy_init( &entropy );

    if( ( ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy,(const unsigned char *) pers,
            strlen( pers ) ) ) != 0 )
    {
        return olen;
    }

    mbedtls_pk_context pk;
    mbedtls_pk_init(&pk);
    if( ( ret = mbedtls_pk_parse_public_key( &pk,(unsigned char*)pdrlPublicKey ,strlen(pdrlPublicKey)+1) ) != 0 )
    {
        goto exit;
    }
    if( ( ret = mbedtls_pk_encrypt( &pk,
            (const unsigned char *)hashBuff,
            32,
            (unsigned char *)outBuff,
            &olen, outBuffSize,
            mbedtls_ctr_drbg_random,
            &ctr_drbg ) ) != 0 )
    {
        goto exit;
    }

    exit:
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );
    return olen;
}

void AP_PDRL_Logger::clearLogFile()
{
	char currentFileName[42] = {0};

	memcpy(&currentFileName,&write_filename,36);
	AP::FS().unlink((char*)currentFileName);

	strcat(currentFileName,".json");
	AP::FS().unlink((char*)currentFileName);

	m_isLogGenerated = 0;

}

void AP_PDRL_Logger::signLogFile(char* inFilePath,size_t len)
{
#if 0
	unsigned char hash[32];
	unsigned char buf[MBEDTLS_MPI_MAX_SIZE];
	int ret = 0;
	//	char *buffer, *base64_out, *der_sign_base64;
	//	uint16_t outlen,der_sign_base64_len;
	//
	//	size_t fSize = fileSize(inFilePath);
	//	buffer = (char *)malloc(*fSize+1);

	//	int byRead = AP::FS().read(logFileFd,buffer,fSize);

	if( ( ret = mbedtls_md_file(mbedtls_md_info_from_type( MBEDTLS_MD_SHA256 ),inFilePath, hash ) ) != 0 )
	{
		return;
	}

	if( ( ret = mbedtls_rsa_pkcs1_sign( AP_KEYSTORE::getInstance()->rsaPtr, NULL, NULL, MBEDTLS_RSA_PRIVATE, MBEDTLS_MD_SHA256,
			20, hash, buf ) ) != 0 )
	{
		return;
	}
	printf("Sig %s",buf);
#endif
}

void AP_PDRL_Logger::logStrToFile(char* logBuff,size_t len)
{
   uint16_t year;
    uint8_t month, day, hour, min, sec;
    uint16_t ms;
    char datetimeBuf[50];

    // First, try to get local time from RTC
    if (AP::rtc().get_local_date_time(year, month, day, hour, min, sec, ms)) {
        // Format the local date and time as "YYYY-MM-DD HH:MM:SS.mmm"
    	month+=1;
        snprintf(datetimeBuf, sizeof(datetimeBuf), "%04u-%02u-%02u %02u:%02u:%02u.%03u",
                 year, month, day, hour, min, sec, ms);
      //  gcs().send_text(AGPILOT_SEVERITY_INFO, "Using RTC Time: %s", datetimeBuf);
    }
    else
    {
        // If RTC time is not available, fallback to GPS time
        uint64_t gps_time_usec = AP::gps().time_epoch_usec();
        if (gps_time_usec != 0) {
            // Convert microseconds to seconds for easier handling
            time_t gps_time_sec = gps_time_usec / 1000000;

            // Define the offset for IST (UTC + 5:30)
            int64_t ist_offset_sec = 5 * 3600 + 30 * 60;

            // Calculate IST time
            time_t ist_time_sec = gps_time_sec + ist_offset_sec;

            // Convert IST time to struct tm
            struct tm *ist_tm = gmtime(&ist_time_sec);

            // Format the date and time as "YYYY-MM-DD HH:MM:SS"
            strftime(datetimeBuf, sizeof(datetimeBuf), "%Y-%m-%d %H:%M:%S", ist_tm);

           // gcs().send_text(AGPILOT_SEVERITY_INFO, "Using GPS Time: %s", datetimeBuf);
        }
        else
        {
            // Fallback if neither RTC nor GPS time is available
            strcpy(datetimeBuf, "No valid time available");
           // gcs().send_text(AGPILOT_SEVERITY_INFO, "Failed to retrieve both RTC and GPS time");
        }
    }

    // Prepare the log line with the obtained date and time
    char lineBuf[200];
    snprintf(lineBuf, sizeof(lineBuf), "[%s] %s", datetimeBuf, logBuff);

    // Write to the log file
	int logFileFd = AP::FS().open("BOOTLOGS.txt",O_APPEND | O_CREAT | O_WRONLY);
	AP::FS().write(logFileFd,lineBuf,strlen(lineBuf));
	AP::FS().close(logFileFd);
}

char *AP_PDRL_Logger::_log_file_name_short(const uint16_t log_num) const
{
    char *buf = nullptr;
    if (asprintf(&buf, "%s/%u.BIN", HAL_BOARD_LOG_DIRECTORY, (unsigned)log_num) == -1) {
        return nullptr;
    }
    return buf;
}

/*
  construct a log file name given a log number.
  The number in the log filename will be zero-padded.
  Note: Caller must free.
 */
char *AP_PDRL_Logger::_log_file_name_long(const uint16_t log_num) const
{
    char *buf = nullptr;
    if (asprintf(&buf, "%s/%08u.BIN", HAL_BOARD_LOG_DIRECTORY, (unsigned)log_num) == -1) {
        return nullptr;
    }
    return buf;
}

/*
  return a log filename appropriate for the supplied log_num if a
  filename exists with the short (not-zero-padded name) then it is the
  appropirate name, otherwise the long (zero-padded) version is.
  Note: Caller must free.
 */
char *AP_PDRL_Logger::_log_file_name(const uint16_t log_num) const
{
    char *filename = _log_file_name_short(log_num);
    if (filename == nullptr) {
        return nullptr;
    }
    if (file_exists(filename)) {
        return filename;
    }
    free(filename);
    return _log_file_name_long(log_num);
}

