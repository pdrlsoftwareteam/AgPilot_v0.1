/*
 * AG_NPNT_data_helper.cpp
 *
 *  Created on: 01-Nov-2019
 *      Author: owner
 */

#include "AG_NPNT_data_helper.h"

AG_NPNT_data_helper* AG_NPNT_data_helper::m_AG_NPNT_data_helper = 0;

AG_NPNT_data_helper::AG_NPNT_data_helper()
{

}

AG_NPNT_data_helper *AG_NPNT_data_helper::getInstance()
{
	if(m_AG_NPNT_data_helper == 0)
		m_AG_NPNT_data_helper = new AG_NPNT_data_helper();
	return m_AG_NPNT_data_helper;
}
