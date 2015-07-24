#include "graphDB.hpp"
#include <string.h>
#include <cstdlib>
#include "node.hpp"

using namespace aeon;

graphDB::graphDB( void )
{

	return ;
}

graphDB::graphDB( sstr db_name, uint db_size )
{
	this->db_ptr = NULL;
	this->create_db(db_name, db_size);
	this->number_of_types = 1000; // that let the first 1000 type id for internal purposes
	this->_add_node_type("_list_elem", 2, {"next", "value"}); // define AEON_LIST_NEXT 1/AEON_LIST_VALUE 2
	/* data type container */
	this->_add_node_type("_type_list", 1, {"list"});
	/* */
	this->_add_node_type("_type", 2, {"concat_fields_names", "name"});
	this->type_data = this->_create_node("_type_list"); //creat the type data container
	/* init */
	// type_data = find type
	// 

	return ;
}

graphDB::graphDB( sstr db_name )
{
	this->db_ptr = NULL;
	this->connect_db(db_name );
	this->number_of_types = 1000; // that let the first 1000 type id for internal purposes
	this->_add_node_type("_list_elem", 2, {"next", "value"}); 
	/* data type container */
	this->_add_node_type("_type_list", 1, {"list"});
	/* */
	this->_add_node_type("_type", 2, {"concat_fields_names", "name"});
	this->type_data = this->_create_node("_type_list"); //creat the type data container
	/* init */
	// type_data = find type
	//

	return ;
}

graphDB::~graphDB( void )
{

	return ;
}

void		graphDB::create_db(sstr db_name, uint db_size)
{
	char *name = new char[db_name.size()+1];

	if(this->db_ptr != NULL)
	{
		std::cout << "ERROR: you can't connect twice to a database" << std::endl;
		exit(-1);
	}
	strcpy(name, db_name.c_str());

	if (!(this->db_ptr = wg_attach_database(name, db_size)))
	{
		std::cout << "Failed to attach database" << std::endl;
		exit(-1);
	}

	delete name;
}


void		graphDB::connect_db(sstr db_name)
{
	char *name = new char[db_name.size()+1];

	if(this->db_ptr != NULL)
	{
		std::cout << "ERROR: you can't connect twice to a database" << std::endl;
		exit(-1);
	}
	strcpy(name, db_name.c_str());

	if (!(this->db_ptr = wg_attach_existing_database(name)))
	{
		std::cout << "Database named \"" << db_name << "failled to attach !" << std::endl;
		exit(-1);
	}

	delete name;
}

void		graphDB::add_node_type(sstr name, uint size, std::vector<sstr> fields_name)
{
	std::map<sstr, uint>	map_fields_name;
/*	node *node1;
	node *node2;
	node *node3;
*/
	if(name[0] == '_')
	{
		std::cout << "/!\\ Type name begining by '_' are reserved." << std::endl;
		return ;
	}
	this->get_type_id[name] = this->number_of_types;
	this->get_type_size[name] = size;
	this->get_type_name[this->number_of_types] = name;

	for (uint i = 0; i < size; i++)
		map_fields_name[fields_name[i]] = i + 1;

	this->get_type_fields[this->number_of_types] = fields_name;
	this->get_type_fields_map[this->number_of_types] = map_fields_name;
	

	/* add type to shared mem */
	//node1 = this->create_node("_type"); //creat the type data container
/*
	node2 = this->create_node("_type"); //  create a new
	node2->set_field("concat field_list", name);
	node2->set_field("name", name);
	for ()
	{
	node2->add_list_elem("name_list", );
	type_data->add_list_elem( "list", node2 ); // add a node in the unique fied
	}
*/
	this->number_of_types += 1; 
}

void		graphDB::_add_node_type(sstr name, uint size, std::vector<sstr> fields_name)
{
	std::map<sstr, uint>	map_fields_name;

	this->get_type_id[name] = this->number_of_types;
	this->get_type_size[name] = size;
	this->get_type_name[this->number_of_types] = name;
	
	for (uint i = 0; i < size; i++)
		map_fields_name[fields_name[i]] = i + 1;

	this->get_type_fields[this->number_of_types] = fields_name;
	this->get_type_fields_map[this->number_of_types] = map_fields_name;

	this->number_of_types += 1; 
}

node		*graphDB::create_node(sstr type_name)
{
	node	*nnode = NULL;
	wg_int	encoded_data;
	void	*rec;

	if(type_name[0] == '_')
	{
		std::cout << "/!\\ Type name begining by '_' are reserved." << std::endl;
		exit(0);
	}
	rec = wg_create_record(this->db_ptr, get_type_size[type_name] + 1); // remember +1 for type field
	if(rec == NULL)
	{
		std::cout << "ERROR: couln't add a new record to the database." << std::endl;
		exit(-1);
	}
	/* set the type field */
        encoded_data = wg_encode_int(db_ptr, get_type_id[type_name]);
        if(encoded_data == WG_ILLEGAL)
        {
                std::cout << "/!\\ Shouldn't happen /!\\" << std::endl;
        }
        if (wg_set_field(db_ptr, rec, 0, encoded_data) < 0)
        {
                std::cout << "Impossible to write in the field" << std::endl;
        }
	/* */
	nnode = new node(rec, this);
	for (uint i = 0; i < get_type_size[type_name]; i++)
		nnode->get_field_index[(get_type_fields[get_type_id[type_name]])[i]] = i + 1;
	
	
	/* don't forget to add type field !!!!! */
	return ( nnode );
}

node		*graphDB::_create_node(sstr type_name)
{
	node	*nnode = NULL;
	wg_int	encoded_data;
	void	*rec;

	rec = wg_create_record(this->db_ptr, get_type_size[type_name] + 1); // remember +1 for type field
	if(rec == NULL)
	{
		std::cout << "ERROR: couln't add a new record to the database." << std::endl;
		exit(-1);
	}
	/* set the type field */
        encoded_data = wg_encode_int(db_ptr, get_type_id[type_name]);
        if(encoded_data == WG_ILLEGAL)
        {
                std::cout << "/!\\ Shouldn't happen /!\\" << std::endl;
        }
        if (wg_set_field(db_ptr, rec, 0, encoded_data) < 0)
        {
                std::cout << "Impossible to write in the field" << std::endl;
        }
	/* */
	nnode = new node(rec, this);
//	for (uint i = 0; i < get_type_size[type_name]; i++)
//		nnode->get_field_index[(get_type_fields[get_type_id[type_name]])[i]] = i + 1;
	
	/* don't forget to add type field !!!!! */
	return ( nnode );
}
node	*graphDB::search_node(sstr type_name, sstr field_name, sstr searched)
{
	void	*rec;
	char *searched_cstr = new char[searched.size()+1];

	strcpy(searched_cstr, searched.c_str());
	wg_int fieldnr = (get_type_fields_map[get_type_id[type_name]])[field_name];
	rec = wg_find_record_str(db_ptr, fieldnr, WG_COND_EQUAL, searched_cstr, NULL);
	delete searched_cstr;
	if (rec == NULL)
		return (NULL);
	else
		return (new node(rec, this));
}
