/*
 * Model.c
 *
 *  Created on: 4 cze 2016
 *      Author: krystian
 */
#include "Model.h"

Model::Model(void)
{
	mysql_init(&this->connection);
}

bool Model::connect(string host, string user, string password, string db)
{
	if(!mysql_real_connect(&this->connection, host.c_str(), user.c_str(), password.c_str(), db.c_str(), 0, NULL, 0))
	{
	    fprintf(stderr, "Failed to connect to database: Error: %s\n", mysql_error(&this->connection));
	    return false;
	}

	return true;
}
