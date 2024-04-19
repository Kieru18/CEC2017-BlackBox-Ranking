CREATE DATABASE zpr_example_database;
grant all privileges on zpr_example_database.* to 'zpr_user'@'localhost';
flush privileges;
create table zpr_example_table (id int primary key, name text);
insert into zpr_example_table (id, name) values(1, 'adrian');
insert into zpr_example_table (id, name) values(2, 'jerzy');
insert into zpr_example_table (id, name) values(3, 'florian');
insert into zpr_example_table (id, name) values(4, 'anna');
insert into zpr_example_table (id, name) values(5, 'czarek');
