CREATE DATABASE zpr_example_database;
grant all privileges on zpr_example_database.* to 'zpr_user'@'localhost';
flush privileges;
CREATE TABLE zpr_users_table (
    mail TEXT,
    hashed_api_key TEXT,
    spend INT,
    PRIMARY KEY (mail(255))
);
CREATE TABLE zpr_request_table (
    mail TEXT,
    PRIMARY KEY (mail(255))
);