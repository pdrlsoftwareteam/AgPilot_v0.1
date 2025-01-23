/*
 * AP_NPNT_data_helper.cpp
 *
 *  Created on: 01-Nov-2019
 *      Author: owner
 */

#include "AP_NPNT_data_helper.h"

AP_NPNT_data_helper* AP_NPNT_data_helper::m_AP_NPNT_data_helper = 0;

AP_NPNT_data_helper::AP_NPNT_data_helper()
{

}

AP_NPNT_data_helper *AP_NPNT_data_helper::getInstance()
{
	if(m_AP_NPNT_data_helper == 0)
		m_AP_NPNT_data_helper = new AP_NPNT_data_helper();
	return m_AP_NPNT_data_helper;
}
