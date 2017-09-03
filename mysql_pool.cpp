#include <acl-lib/acl_cpp/lib_acl.hpp>

static void tbl_select(acl::db_handle& db)
{
	const char* username = "user1";
	acl::query query;
	query.create_sql("select nick from user_tbl where user=:user")
		.set_parameter("user", username);
	if (db.exec_select(query) == false) {
		printf("sql(%s) error\r\n", query.to_string().c_str());
		return;
	}

	for (size_t i = 0; i < db.length(); i++) {
		const acl::db_row* row = db[i];
		const char* nick = (*row)["nick"];
		printf("user(%s)'s nick is %s\r\n", username, nick ? nick : "null");
	}
}

static void tbl_select(acl::db_pool& dbpool)
{
	acl::db_guard guard(dbpool);
	acl::db_handle* db = (acl::db_handle*) guard.peek();
	if (db == NULL) {
		printf("no db connection available!\r\n");
		return;
	}

	tbl_select(*db);
}

int main(void)
{
	acl::acl_cpp_init();
	acl::log::stdout_open(true);

	const char* libpath = "./libmysqlclient_r.so";
	acl::db_handle::set_loadpath(libpath);

	const char* dbaddr = "127.0.0.1:3306", *dbname = "acl_db",
	      *dbuser = "db_user", *dbpass = "111111";
	int dblimit = 10;
	acl::mysql_conf dbconf(dbaddr, dbname);

	dbconf.set_dbuser(dbuser).set_dbpass(dbpass).set_dblimit(dblimit);
	acl::mysql_pool dbpool(dbconf);
	tbl_select(dbpool);

	return 0;
}
