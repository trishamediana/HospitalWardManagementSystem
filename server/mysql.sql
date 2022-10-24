create table rooms
(
    name     int null,
    id       int auto_increment,
    capacity int null,
    constraint id
        unique (id)
);

create table status
(
    id int auto_increment
        primary key
);

create table beds
(
    id        int auto_increment
        primary key,
    name      int null,
    status_id int not null,
    constraint status_id
        foreign key (status_id) references status (id)
            on delete cascade
);

