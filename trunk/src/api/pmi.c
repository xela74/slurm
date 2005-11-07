/*****************************************************************************\
 *  pmi.c - Process Management Interface for MPICH2
 *  See http://www-unix.mcs.anl.gov/mpi/mpich2/
 *
 *  NOTE: Dynamic Process Management functions (PMI part 2) are not supported
 *  at this time. Functions required for MPI-1 (PMI part 1) are supported.
 *****************************************************************************
 *  COPYRIGHT: For the function definitions
 *
 *  The following is a notice of limited availability of the code, and
 *  disclaimer which must be included in the prologue of the code and in all
 *  source listings of the code.
 *
 *  Copyright Notice + 2002 University of Chicago
 *
 *  Permission is hereby granted to use, reproduce, prepare derivative
 *  works, and to redistribute to others. This software was authored by:
 *
 *  Argonne National Laboratory Group
 *  W. Gropp: (630) 252-4318; FAX: (630) 252-5986; e-mail: gropp@mcs.anl.gov
 *  E. Lusk: (630) 252-7852; FAX: (630) 252-5986; e-mail: lusk@mcs.anl.gov
 *  Mathematics and Computer Science Division Argonne National Laboratory,
 *  Argonne IL 60439
 *
 *  GOVERNMENT LICENSE
 *
 *  Portions of this material resulted from work developed under a U.S.
 *  Government Contract and are subject to the following license: the
 *  Government is granted for itself and others acting on its behalf a
 *  paid-up, nonexclusive, irrevocable worldwide license in this computer
 *  software to reproduce, prepare derivative works, and perform publicly
 *  and display publicly.
 *
 *  DISCLAIMER
 *
 *  This computer code material was prepared, in part, as an account of work
 *  sponsored by an agency of the United States Government. Neither the
 *  United States, nor the University of Chicago, nor any of their
 *  employees, makes any warranty express or implied, or assumes any legal
 *  liability or responsibility for the accuracy, completeness, or
 *  usefulness of any information, apparatus, product, or process disclosed,
 *  or represents that its use would not infringe privately owned rights.
 *
 *  MCS Division <http://www.mcs.anl.gov>        Argonne National Laboratory
 *  <http://www.anl.gov>     University of Chicago <http://www.uchicago.edu>
 *****************************************************************************
 *  COPYRIGHT: For the implementation of the functions
 *
 *  Copyright (C) 2005 The Regents of the University of California.
 *  Produced at Lawrence Livermore National Laboratory (cf, DISCLAIMER).
 *  Written by Morris Jette <jette1@llnl.gov>
 *  UCRL-CODE-2002-040.
 *
 *  This file is part of SLURM, a resource management program.
 *  For details, see <http://www.llnl.gov/linux/slurm/>.
 *
 *  SLURM is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  SLURM is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  SLURM is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  SLURM is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with SLURM; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
\*****************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include <slurm/pmi.h>

/* These values should probaby be moved to a location easily 
 * accessible to other SLURM tools. */
#define PMI_MAX_GRP_ID_LEN 32	/* Maximum size of a process group ID */
#define PMI_MAX_KEY_LEN 256	/* Maximum size of a PMI key */
#define PMI_MAX_KVSNAME_LEN 256	/* Maximum size of KVS name */
#define PMI_MAX_VAL_LEN  256	/* Maximum size of a PMI value */

/* Global variables */
int pmi_init;
int pmi_size;
int pmi_spawned;
int pmi_rank;

/* PMI Group functions */

/*@
PMI_Init - initialize the Process Manager Interface

Output Parameter:
. spawned - spawned flag

Return values:
+ PMI_SUCCESS - initialization completed successfully
. PMI_ERR_INVALID_ARG - invalid argument
- PMI_FAIL - initialization failed

Notes:
Initialize PMI for this process group. The value of spawned indicates whether
this process was created by 'PMI_Spawn_multiple'.  'spawned' will be 'PMI_TRUE' 
if this process group has a parent and 'PMI_FALSE' if it does not.

@*/
int PMI_Init( int *spawned )
{
	char *env;

	if (spawned == NULL)
		return PMI_ERR_INVALID_ARG;

	if (pmi_init)
		goto replay;

	env = getenv("PMI_SPAWNED");
	if (env)
		pmi_spawned = atoi(env);
	else
		pmi_spawned = 0;

	env = getenv("SLURM_NPROCS");
	if (!env)
		env = getenv("PMI_SIZE");
	if (env)
		pmi_size = atoi(env);
	else
		pmi_size = 1;

	env = getenv("SLURM_PROCID");
	if (!env)
		env = getenv("PMI_RANK");
	if (env)
		pmi_rank = atoi(env);
	else
		pmi_rank = 0;

	pmi_init = 1;

replay:	if (pmi_spawned)
		*spawned = PMI_TRUE;
	else
		*spawned = PMI_FALSE;
	return PMI_SUCCESS;
}

/*@
PMI_Initialized - check if PMI has been initialized

Output Parameter:
. initialized - boolean value

Return values:
+ PMI_SUCCESS - initialized successfully set
. PMI_ERR_INVALID_ARG - invalid argument
- PMI_FAIL - unable to set the variable

Notes:
On successful output, initialized will either be 'PMI_TRUE' or 'PMI_FALSE'.

+ PMI_TRUE - initialize has been called.
- PMI_FALSE - initialize has not been called or previously failed.

@*/
int PMI_Initialized( PMI_BOOL *initialized )
{
	if (initialized == NULL)
		return PMI_ERR_INVALID_ARG;

	if (pmi_init)
		*initialized = PMI_TRUE;
	else
		*initialized = PMI_FALSE;

	return PMI_SUCCESS;
}

/*@
PMI_Finalize - finalize the Process Manager Interface

Return values:
+ PMI_SUCCESS - finalization completed successfully
- PMI_FAIL - finalization failed

Notes:
 Finalize PMI for this process group.

@*/
int PMI_Finalize( void )
{
	pmi_init = 0;

	return PMI_SUCCESS;
}

/*@
PMI_Get_size - obtain the size of the process group

Output Parameters:
. size - pointer to an integer that receives the size of the process group

Return values:
+ PMI_SUCCESS - size successfully obtained
. PMI_ERR_INVALID_ARG - invalid argument
- PMI_FAIL - unable to return the size

Notes:
This function returns the size of the process group to which the local process
belongs.

@*/
int PMI_Get_size( int *size )
{
	if (size == NULL)
		return PMI_ERR_INVALID_ARG;

	if (!pmi_init) {
		int spawned;
		PMI_Init(&spawned);
		if (!pmi_init)
			return PMI_FAIL;
	}
 
	*size = pmi_size;
	return PMI_SUCCESS;
}

/*@
PMI_Get_rank - obtain the rank of the local process in the process group

Output Parameters:
. rank - pointer to an integer that receives the rank in the process group

Return values:
+ PMI_SUCCESS - rank successfully obtained
. PMI_ERR_INVALID_ARG - invalid argument
- PMI_FAIL - unable to return the rank

Notes:
This function returns the rank of the local process in its process group.

@*/
int PMI_Get_rank( int *rank )
{
	if (rank == NULL)
		return PMI_ERR_INVALID_ARG;

	if (!pmi_init) {
		int spawned;
		PMI_Init(&spawned);
		if (!pmi_init)
			return PMI_FAIL;
	}

	*rank = pmi_rank;
	return PMI_SUCCESS;
}

/*@
PMI_Get_universe_size - obtain the universe size
(NOTE: "universe size" indicates the maximum recommended 
process count for the job.)

Output Parameters:
. size - pointer to an integer that receives the size

Return values:
+ PMI_SUCCESS - size successfully obtained
. PMI_ERR_INVALID_ARG - invalid argument
- PMI_FAIL - unable to return the size


@*/
int PMI_Get_universe_size( int *size )
{
	char *env;

	if (size == NULL)
		return PMI_ERR_INVALID_ARG;

	env = getenv("SLURM_NPROCS");
	if (!env)
		env = getenv("PMI_SIZE");
	if (env) {
		*size = atoi(env);
		return PMI_SUCCESS;
	}

	env = getenv("SLURM_NNODES");
	if (env) {
		/* FIXME: We want a processor count here */
		*size = atoi(env);
		return PMI_SUCCESS;
	}

	*size = 1;
	return PMI_SUCCESS;
}

/*@
PMI_Get_appnum - obtain the application number

Output parameters:
. appnum - pointer to an integer that receives the appnum

Return values:
+ PMI_SUCCESS - appnum successfully obtained
. PMI_ERR_INVALID_ARG - invalid argument
- PMI_FAIL - unable to return the size


@*/
int PMI_Get_appnum( int *appnum )
{
	if (appnum == NULL)
		return PMI_ERR_INVALID_ARG;

	/* FIXME: What is "application number"? */
	return PMI_FAIL;
}

/*@
PMI_Publish_name - publish a name 

Input parameters:
. service_name - string representing the service being published
. port - string representing the port on which to contact the service

Return values:
+ PMI_SUCCESS - port for service successfully published
. PMI_ERR_INVALID_ARG - invalid argument
- PMI_FAIL - unable to publish service


@*/
int PMI_Publish_name( const char service_name[], const char port[] )
{
	if ((service_name == NULL) || (port == NULL))
		return PMI_ERR_INVALID_ARG;

	/* FIXME */
	return PMI_FAIL;
}

/*@
PMI_Unpublish_name - unpublish a name

Input parameters:
. service_name - string representing the service being unpublished

Return values:
+ PMI_SUCCESS - port for service successfully published
. PMI_ERR_INVALID_ARG - invalid argument
- PMI_FAIL - unable to unpublish service


@*/
int PMI_Unpublish_name( const char service_name[] )
{
	if (service_name == NULL)
		return PMI_ERR_INVALID_ARG;

	/* FIXME */
	return PMI_FAIL;
}

/*@
PMI_Lookup_name - lookup a service by name

Input parameters:
. service_name - string representing the service being published

Output parameters:
. port - string representing the port on which to contact the service

Return values:
+ PMI_SUCCESS - port for service successfully obtained
. PMI_ERR_INVALID_ARG - invalid argument
- PMI_FAIL - unable to lookup service


@*/
int PMI_Lookup_name( const char service_name[], char port[] )
{
	if ((service_name == NULL) || (port == NULL))
		return PMI_ERR_INVALID_ARG;

	/* FIXME */
	return PMI_FAIL;
}

/*@
PMI_Get_id - obtain the id of the process group

Input Parameter:
. length - length of the id_str character array

Output Parameter:
. id_str - character array that receives the id of the process group

Return values:
+ PMI_SUCCESS - id successfully obtained
. PMI_ERR_INVALID_ARG - invalid rank argument
. PMI_ERR_INVALID_LENGTH - invalid length argument
- PMI_FAIL - unable to return the id

Notes:
This function returns a string that uniquely identifies the process group
that the local process belongs to.  The string passed in must be at least
as long as the number returned by 'PMI_Get_id_length_max()'.

@*/
int PMI_Get_id( char id_str[], int length )
{
	char *env;
	uint32_t job_id, step_id;

	if (id_str == NULL)
		return PMI_ERR_INVALID_ARG;
	if (length < PMI_MAX_GRP_ID_LEN)
		return PMI_ERR_INVALID_LENGTH;

	env = getenv("SLURM_JOBID");
	if (env)
		job_id = atoi(env);
	else
		return PMI_FAIL;

	env = getenv("SLURM_STEPID");
	if (env) {
		step_id = atoi(env);
		snprintf(id_str, length, "%u.%u", job_id, step_id);
	} else
		snprintf(id_str, length, "%u", job_id);	

	return PMI_SUCCESS;
}

/*@
PMI_Get_kvs_domain_id - obtain the id of the PMI domain

Input Parameter:
. length - length of id_str character array

Output Parameter:
. id_str - character array that receives the id of the PMI domain

Return values:
+ PMI_SUCCESS - id successfully obtained
. PMI_ERR_INVALID_ARG - invalid argument
. PMI_ERR_INVALID_LENGTH - invalid length argument
- PMI_FAIL - unable to return the id

Notes:
This function returns a string that uniquely identifies the PMI domain
where keyval spaces can be shared.  The string passed in must be at least
as long as the number returned by 'PMI_Get_id_length_max()'.

@*/
int PMI_Get_kvs_domain_id( char id_str[], int length )
{
	if (id_str == NULL)
		return PMI_ERR_INVALID_ARG;

	/* FIXME */
	return PMI_FAIL;
}

/*@
PMI_Get_id_length_max - obtain the maximum length of an id string

Output Parameters:
. length - the maximum length of an id string

Return values:
+ PMI_SUCCESS - length successfully set
. PMI_ERR_INVALID_ARG - invalid argument
- PMI_FAIL - unable to return the maximum length

Notes:
This function returns the maximum length of a process group id string.

@*/
int PMI_Get_id_length_max( int *length )
{
	if (length == NULL)
		return PMI_ERR_INVALID_ARG;

	*length = PMI_MAX_GRP_ID_LEN;
	return PMI_FAIL;
}

/*@
PMI_Barrier - barrier across the process group

Return values:
+ PMI_SUCCESS - barrier successfully finished
- PMI_FAIL - barrier failed

Notes:
This function is a collective call across all processes in the process group
the local process belongs to.  It will not return until all the processes
have called 'PMI_Barrier()'.

@*/
int PMI_Barrier( void )
{
	/* FIXME */
	return PMI_FAIL;
}

/*@
PMI_Get_clique_size - obtain the number of processes on the local node

Output Parameters:
. size - pointer to an integer that receives the size of the clique

Return values:
+ PMI_SUCCESS - size successfully obtained
. PMI_ERR_INVALID_ARG - invalid argument
- PMI_FAIL - unable to return the clique size

Notes:
This function returns the number of processes in the local process group that
are on the local node along with the local process.  This is a simple topology
function to distinguish between processes that can communicate through IPC
mechanisms (e.g., shared memory) and other network mechanisms.

@*/
int PMI_Get_clique_size( int *size )
{
	if (size == NULL)
		return PMI_ERR_INVALID_ARG;

	/* FIXME */
	return PMI_FAIL;
}

/*@
PMI_Get_clique_ranks - get the ranks of the local processes in the process group

Input Parameters:
. length - length of the ranks array

Output Parameters:
. ranks - pointer to an array of integers that receive the local ranks

Return values:
+ PMI_SUCCESS - ranks successfully obtained
. PMI_ERR_INVALID_ARG - invalid argument
. PMI_ERR_INVALID_LENGTH - invalid length argument
- PMI_FAIL - unable to return the ranks

Notes:
This function returns the ranks of the processes on the local node.  The array
must be at least as large as the size returned by 'PMI_Get_clique_size()'.  This
is a simple topology function to distinguish between processes that can
communicate through IPC mechanisms (e.g., shared memory) and other network
mechanisms.

@*/
int PMI_Get_clique_ranks( int ranks[], int length )
{
	if (ranks == NULL)
		return PMI_ERR_INVALID_ARG;

	/* FIXME */
	return PMI_FAIL;
}

/*@
PMI_Abort - abort the process group associated with this process

Input Parameters:
+ exit_code - exit code to be returned by this process
- error_msg - error message to be printed

Return values:
. none - this function should not return
@*/
int PMI_Abort(int exit_code, const char error_msg[])
{
	/* FIXME */
	return PMI_FAIL;
}

/* PMI Keymap functions */
/*@
PMI_KVS_Get_my_name - obtain the name of the keyval space the local process group has access to

Input Parameters:
. length - length of the kvsname character array

Output Parameters:
. kvsname - a string that receives the keyval space name

Return values:
+ PMI_SUCCESS - kvsname successfully obtained
. PMI_ERR_INVALID_ARG - invalid argument
. PMI_ERR_INVALID_LENGTH - invalid length argument
- PMI_FAIL - unable to return the kvsname

Notes:
This function returns the name of the keyval space that this process and all
other processes in the process group have access to.  The output parameter,
kvsname, must be at least as long as the value returned by
'PMI_KVS_Get_name_length_max()'.

@*/
int PMI_KVS_Get_my_name( char kvsname[], int length )
{
	if (kvsname == NULL)
		return PMI_ERR_INVALID_ARG;
	if (length < PMI_MAX_KVSNAME_LEN)
		return PMI_ERR_INVALID_LENGTH;

	/* FIXME */
	return PMI_FAIL;
}

/*@
PMI_KVS_Get_name_length_max - obtain the length necessary to store a kvsname

Output Parameter:
. length - maximum length required to hold a keyval space name

Return values:
+ PMI_SUCCESS - length successfully set
. PMI_ERR_INVALID_ARG - invalid argument
- PMI_FAIL - unable to set the length

Notes:
This function returns the string length required to store a keyval space name.

A routine is used rather than setting a maximum value in 'pmi.h' to allow
different implementations of PMI to be used with the same executable.  These
different implementations may allow different maximum lengths; by using a 
routine here, we can interface with a variety of implementations of PMI.

@*/
int PMI_KVS_Get_name_length_max( int *length )
{
	if (length == NULL)
		return PMI_ERR_INVALID_ARG;

	*length = PMI_MAX_KVSNAME_LEN;
	return PMI_SUCCESS;
}

/*@
PMI_KVS_Get_key_length_max - obtain the length necessary to store a key

Output Parameter:
. length - maximum length required to hold a key string.

Return values:
+ PMI_SUCCESS - length successfully set
. PMI_ERR_INVALID_ARG - invalid argument
- PMI_FAIL - unable to set the length

Notes:
This function returns the string length required to store a key.

@*/
int PMI_KVS_Get_key_length_max( int *length )
{
	if (length == NULL)
		return PMI_ERR_INVALID_ARG;

	*length = PMI_MAX_KEY_LEN;
	return PMI_SUCCESS;
}

/*@
PMI_KVS_Get_value_length_max - obtain the length necessary to store a value

Output Parameter:
. length - maximum length required to hold a keyval space value

Return values:
+ PMI_SUCCESS - length successfully set
. PMI_ERR_INVALID_ARG - invalid argument
- PMI_FAIL - unable to set the length

Notes:
This function returns the string length required to store a value from a
keyval space.

@*/
int PMI_KVS_Get_value_length_max( int *length )
{
	if (length == NULL)
		return PMI_ERR_INVALID_ARG;

	*length = PMI_MAX_VAL_LEN;
	return PMI_SUCCESS;
}

/*@
PMI_KVS_Create - create a new keyval space

Input Parameter:
. length - length of the kvsname character array

Output Parameters:
. kvsname - a string that receives the keyval space name

Return values:
+ PMI_SUCCESS - keyval space successfully created
. PMI_ERR_INVALID_ARG - invalid argument
. PMI_ERR_INVALID_LENGTH - invalid length argument
- PMI_FAIL - unable to create a new keyval space

Notes:
This function creates a new keyval space.  Everyone in the same process group
can access this keyval space by the name returned by this function.  The
function is not collective.  Only one process calls this function.  The output
parameter, kvsname, must be at least as long as the value returned by
'PMI_KVS_Get_name_length_max()'.

@*/
int PMI_KVS_Create( char kvsname[], int length )
{
	if (kvsname == NULL)
		return PMI_ERR_INVALID_ARG;
	if (length < PMI_MAX_KVSNAME_LEN)
		return PMI_ERR_INVALID_LENGTH;

	/* FIXME */
	return PMI_FAIL;
}

/*@
PMI_KVS_Destroy - destroy keyval space

Input Parameters:
. kvsname - keyval space name

Return values:
+ PMI_SUCCESS - keyval space successfully destroyed
. PMI_ERR_INVALID_ARG - invalid argument
- PMI_FAIL - unable to destroy the keyval space

Notes:
This function destroys a keyval space created by 'PMI_KVS_Create()'.

@*/
int PMI_KVS_Destroy( const char kvsname[] )
{
	if (kvsname == NULL)
		return PMI_ERR_INVALID_ARG;

	/* FIXME */
	return PMI_FAIL;
}

/*@
PMI_KVS_Put - put a key/value pair in a keyval space

Input Parameters:
+ kvsname - keyval space name
. key - key
- value - value

Return values:
+ PMI_SUCCESS - keyval pair successfully put in keyval space
. PMI_ERR_INVALID_KVS - invalid kvsname argument
. PMI_ERR_INVALID_KEY - invalid key argument
. PMI_ERR_INVALID_VAL - invalid val argument
- PMI_FAIL - put failed

Notes:
This function puts the key/value pair in the specified keyval space.  The
value is not visible to other processes until 'PMI_KVS_Commit()' is called.  
The function may complete locally.  After 'PMI_KVS_Commit()' is called, the
value may be retrieved by calling 'PMI_KVS_Get()'.  All keys put to a keyval
space must be unique to the keyval space.  You may not put more than once
with the same key.

@*/
int PMI_KVS_Put( const char kvsname[], const char key[], const char value[])
{
	if ((kvsname == NULL) || (strlen(kvsname) > PMI_MAX_KVSNAME_LEN))
		return PMI_ERR_INVALID_KVS;
	if ((key == NULL) || (strlen(key) >PMI_MAX_KEY_LEN))
		return PMI_ERR_INVALID_KEY;
	if ((value == NULL) || (strlen(value) > PMI_MAX_VAL_LEN))
		return PMI_ERR_INVALID_VAL;

	/* FIXME */
	return PMI_FAIL;
}

/*@
PMI_KVS_Commit - commit all previous puts to the keyval space

Input Parameters:
. kvsname - keyval space name

Return values:
+ PMI_SUCCESS - commit succeeded
. PMI_ERR_INVALID_ARG - invalid argument
- PMI_FAIL - commit failed

Notes:
This function commits all previous puts since the last 'PMI_KVS_Commit()' into
the specified keyval space. It is a process local operation.

@*/
int PMI_KVS_Commit( const char kvsname[] )
{
	if ((kvsname == NULL) || (strlen(kvsname) > PMI_MAX_KVSNAME_LEN))
		return PMI_ERR_INVALID_ARG;

	/* FIXME */
	return PMI_FAIL;
}

/*@
PMI_KVS_Get - get a key/value pair from a keyval space

Input Parameters:
+ kvsname - keyval space name
. key - key
- length - length of value character array

Output Parameters:
. value - value

Return values:
+ PMI_SUCCESS - get succeeded
. PMI_ERR_INVALID_KVS - invalid kvsname argument
. PMI_ERR_INVALID_KEY - invalid key argument
. PMI_ERR_INVALID_VAL - invalid val argument
. PMI_ERR_INVALID_LENGTH - invalid length argument
- PMI_FAIL - get failed

Notes:
This function gets the value of the specified key in the keyval space.

@*/
int PMI_KVS_Get( const char kvsname[], const char key[], char value[], int length)
{
	if ((kvsname == NULL) || (strlen(kvsname) > PMI_MAX_KVSNAME_LEN))
		return PMI_ERR_INVALID_KVS;
	if ((key == NULL) || (strlen(key) >PMI_MAX_KEY_LEN))
		return PMI_ERR_INVALID_KEY;
	if (value == NULL)
		return PMI_ERR_INVALID_VAL;
	if (length < PMI_MAX_VAL_LEN)
		return PMI_ERR_INVALID_LENGTH;

	/* FIXME */
	return PMI_FAIL;
}

/*@
PMI_KVS_Iter_first - initialize the iterator and get the first value

Input Parameters:
+ kvsname - keyval space name
. key_len - length of key character array
- val_len - length of val character array

Output Parameters:
+ key - key
- value - value

Return values:
+ PMI_SUCCESS - keyval pair successfully retrieved from the keyval space
. PMI_ERR_INVALID_KVS - invalid kvsname argument
. PMI_ERR_INVALID_KEY - invalid key argument
. PMI_ERR_INVALID_KEY_LENGTH - invalid key length argument
. PMI_ERR_INVALID_VAL - invalid val argument
. PMI_ERR_INVALID_VAL_LENGTH - invalid val length argument
- PMI_FAIL - failed to initialize the iterator and get the first keyval pair

Notes:
This function initializes the iterator for the specified keyval space and
retrieves the first key/val pair.  The end of the keyval space is specified
by returning an empty key string.  key and val must be at least as long as
the values returned by 'PMI_KVS_Get_key_length_max()' and
'PMI_KVS_Get_value_length_max()'.

@*/
int PMI_KVS_Iter_first(const char kvsname[], char key[], int key_len, char val[], int val_len)
{
	if ((kvsname == NULL) || (strlen(kvsname) > PMI_MAX_KVSNAME_LEN))
		return PMI_ERR_INVALID_KVS;
	if (key == NULL)
		return PMI_ERR_INVALID_KEY;
	if (key_len < PMI_MAX_KEY_LEN)
		return PMI_ERR_INVALID_KEY_LENGTH;
	if (val == NULL)
		return PMI_ERR_INVALID_VAL;
	if (val_len < PMI_MAX_VAL_LEN)
		return PMI_ERR_INVALID_VAL_LENGTH;

	/* FIXME */
	return PMI_FAIL;
}

/*@
PMI_KVS_Iter_next - get the next keyval pair from the keyval space

Input Parameters:
+ kvsname - keyval space name
. key_len - length of key character array
- val_len - length of val character array

Output Parameters:
+ key - key
- value - value

Return values:
+ PMI_SUCCESS - keyval pair successfully retrieved from the keyval space
. PMI_ERR_INVALID_KVS - invalid kvsname argument
. PMI_ERR_INVALID_KEY - invalid key argument
. PMI_ERR_INVALID_KEY_LENGTH - invalid key length argument
. PMI_ERR_INVALID_VAL - invalid val argument
. PMI_ERR_INVALID_VAL_LENGTH - invalid val length argument
- PMI_FAIL - failed to get the next keyval pair

Notes:
This function retrieves the next keyval pair from the specified keyval space.  
'PMI_KVS_Iter_first()' must have been previously called.  The end of the keyval
space is specified by returning an empty key string.  The output parameters,
key and val, must be at least as long as the values returned by
'PMI_KVS_Get_key_length_max()' and 'PMI_KVS_Get_value_length_max()'.

@*/
int PMI_KVS_Iter_next(const char kvsname[], char key[], int key_len, char val[], int val_len)
{
	if ((kvsname == NULL) || (strlen(kvsname) > PMI_MAX_KVSNAME_LEN))
		return PMI_ERR_INVALID_KVS;
	if (key == NULL)
		return PMI_ERR_INVALID_KEY;
	if (key_len < PMI_MAX_KEY_LEN)
		return PMI_ERR_INVALID_KEY_LENGTH;
	if (val == NULL)
		return PMI_ERR_INVALID_VAL;
	if (val_len < PMI_MAX_VAL_LEN)
		return PMI_ERR_INVALID_VAL_LENGTH;

	/* FIXME */
	return PMI_FAIL;
}

/* PMI Process Creation functions */

/*@
PMI_Spawn_multiple - spawn a new set of processes

Input Parameters:
+ count - count of commands
. cmds - array of command strings
. argvs - array of argv arrays for each command string
. maxprocs - array of maximum processes to spawn for each command string
. info_keyval_sizes - array giving the number of elements in each of the 
  'info_keyval_vectors'
. info_keyval_vectors - array of keyval vector arrays
. preput_keyval_size - Number of elements in 'preput_keyval_vector'
- preput_keyval_vector - array of keyvals to be pre-put in the spawned keyval space

Output Parameter:
. errors - array of errors for each command

Return values:
+ PMI_SUCCESS - spawn successful
. PMI_ERR_INVALID_ARG - invalid argument
- PMI_FAIL - spawn failed

Notes:
This function spawns a set of processes into a new process group.  The 'count'
field refers to the size of the array parameters - 'cmd', 'argvs', 'maxprocs',
'info_keyval_sizes' and 'info_keyval_vectors'.  The 'preput_keyval_size' refers
to the size of the 'preput_keyval_vector' array.  The 'preput_keyval_vector'
contains keyval pairs that will be put in the keyval space of the newly
created process group before the processes are started.  The 'maxprocs' array
specifies the desired number of processes to create for each 'cmd' string.  
The actual number of processes may be less than the numbers specified in
maxprocs.  The acceptable number of processes spawned may be controlled by
``soft'' keyvals in the info arrays.  The ``soft'' option is specified by
mpiexec in the MPI-2 standard.  Environment variables may be passed to the
spawned processes through PMI implementation specific 'info_keyval' parameters.
@*/
int PMI_Spawn_multiple(int count,
                       const char * cmds[],
                       const char ** argvs[],
                       const int maxprocs[],
                       const int info_keyval_sizesp[],
                       const PMI_keyval_t * info_keyval_vectors[],
                       int preput_keyval_size,
                       const PMI_keyval_t preput_keyval_vector[],
                       int errors[])
{
	if (cmds == NULL)
		return PMI_ERR_INVALID_ARG;

	/* FIXME */
	return PMI_FAIL;
}

/*@
PMI_Parse_option - create keyval structures from a single command line argument

Input Parameters:
+ num_args - length of args array
- args - array of command line arguments starting with the argument to be parsed

Output Parameters:
+ num_parsed - number of elements of the argument array parsed
. keyvalp - pointer to an array of keyvals
- size - size of the allocated array

Return values:
+ PMI_SUCCESS - success
. PMI_ERR_INVALID_NUM_ARGS - invalid number of arguments
. PMI_ERR_INVALID_ARGS - invalid args argument
. PMI_ERR_INVALID_NUM_PARSED - invalid num_parsed length argument
. PMI_ERR_INVALID_KEYVALP - invalid keyvalp argument
. PMI_ERR_INVALID_SIZE - invalid size argument
- PMI_FAIL - fail

Notes:
This function removes one PMI specific argument from the command line and
creates the corresponding 'PMI_keyval_t' structure for it.  It returns
an array and size to the caller.  The array must be freed by 'PMI_Free_keyvals()'.
If the first element of the args array is not a PMI specific argument, the 
function returns success and sets num_parsed to zero.  If there are multiple PMI 
specific arguments in the args array, this function may parse more than one 
argument as long as the options are contiguous in the args array.

@*/
int PMI_Parse_option(int num_args, char *args[], int *num_parsed, PMI_keyval_t **keyvalp, int *size)
{
	if (num_parsed == NULL)
		return PMI_ERR_INVALID_NUM_PARSED;
	if (keyvalp == NULL)
		return PMI_ERR_INVALID_KEYVALP;
	if (size == NULL)
		return PMI_ERR_INVALID_SIZE;

	/* FIXME */
	return PMI_FAIL;
}

/*@
PMI_Args_to_keyval - create keyval structures from command line arguments

Input Parameters:
+ argcp - pointer to argc
- argvp - pointer to argv

Output Parameters:
+ keyvalp - pointer to an array of keyvals
- size - size of the allocated array

Return values:
+ PMI_SUCCESS - success
. PMI_ERR_INVALID_ARG - invalid argument
- PMI_FAIL - fail

Notes:
This function removes PMI specific arguments from the command line and
creates the corresponding 'PMI_keyval_t' structures for them.  It returns
an array and size to the caller that can then be passed to 'PMI_Spawn_multiple()'.
The array can be freed by 'PMI_Free_keyvals()'.  The routine 'free()' should 
not be used to free this array as there is no requirement that the array be
allocated with 'malloc()'.

@*/
int PMI_Args_to_keyval(int *argcp, char *((*argvp)[]), PMI_keyval_t **keyvalp, 
		int *size)
{
	if ((keyvalp == NULL) || (size == NULL))
		return PMI_ERR_INVALID_ARG;

	/* FIXME */
	return PMI_FAIL;
}

/*@
PMI_Free_keyvals - free the keyval structures created by PMI_Args_to_keyval

Input Parameters:
+ keyvalp - array of keyvals
- size - size of the array

Return values:
+ PMI_SUCCESS - success
. PMI_ERR_INVALID_ARG - invalid argument
- PMI_FAIL - fail

Notes:
 This function frees the data returned by 'PMI_Args_to_keyval' and 'PMI_Parse_option'.
 Using this routine instead of 'free' allows the PMI package to track 
 allocation of storage or to use interal storage as it sees fit.
@*/
int PMI_Free_keyvals(PMI_keyval_t keyvalp[], int size)
{
	if ((keyvalp == NULL) && size)
		return PMI_ERR_INVALID_ARG;

	/* FIXME */
	return PMI_FAIL;
}

/*@
PMI_Get_options - get a string of command line argument descriptions that may be printed to the user

Input Parameters:
. length - length of str

Output Parameters:
+ str - description string
- length - length of string or necessary length if input is not large enough

Return values:
+ PMI_SUCCESS - success
. PMI_ERR_INVALID_ARG - invalid argument
. PMI_ERR_INVALID_LENGTH - invalid length argument
. PMI_ERR_NOMEM - input length too small
- PMI_FAIL - fail

Notes:
 This function returns the command line options specific to the pmi implementation
@*/
int PMI_Get_options(char *str, int *length)
{
	if ((str == NULL) || (length == NULL))
		return PMI_ERR_INVALID_ARG;

	/* FIXME */
	return PMI_FAIL;
}
