/*-------------------------------------------------------------------------
 *
 * worker_create_truncate_trigger_protocol.c
 *
 * Routines for creating truncate triggers on distributed tables on worker nodes.
 *
 * Copyright (c) 2016, Citus Data, Inc.
 *
 * $Id$
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"
#include "fmgr.h"

#include "distributed/citus_ruleutils.h"
#include "distributed/master_metadata_utility.h"
#include "distributed/metadata_cache.h"
#include "distributed/metadata_sync.h"
#include "utils/elog.h"
#include "utils/fmgroids.h"
#include "utils/lsyscache.h"


PG_FUNCTION_INFO_V1(worker_create_truncate_trigger);


/*
 * worker_create_truncate_trigger creates a truncate trigger for the given distributed
 * table on current metadata worker. The function is intented to be called by the
 * coordinator node during metadata propagation of mx tables. The function requires
 * superuser permissions and fails if this node is not a metadata worker or the specified
 * table is not an mx table.
 */
Datum
worker_create_truncate_trigger(PG_FUNCTION_ARGS)
{
	Oid relationId = PG_GETARG_OID(0);

	EnsureSuperUser();

	/* Ensure that this node is a worker with metadata */
	if (GetLocalGroupId() == 0)
	{
		ereport(ERROR, (errmsg("this node is not a metadata worker")));
	}

	/* Ensure that the specified table is an MX table */
	if (!ShouldSyncTableMetadata(relationId))
	{
		char *tableName = generate_qualified_relation_name(relationId);
		ereport(ERROR, (errmsg("%s is not an mx table", tableName)));
	}

	/* Create the truncate trigger */
	CreateTruncateTrigger(relationId);

	PG_RETURN_VOID();
}
