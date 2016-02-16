-- Table: qgis_test.authors

DROP TABLE IF EXISTS qgis_test.books_authors;
DROP TABLE IF EXISTS qgis_test.authors;
DROP TABLE IF EXISTS qgis_test.books;

CREATE TABLE qgis_test.authors
(
  pk serial NOT NULL,
  name character varying(255),
  CONSTRAINT authors_pkey PRIMARY KEY (pk),
  CONSTRAINT authors_name_key UNIQUE (name)
);

-- Table: qgis_test.books


CREATE TABLE qgis_test.books
(
  pk serial NOT NULL,
  name character varying(255),
  CONSTRAINT books_pkey PRIMARY KEY (pk),
  CONSTRAINT books_name_key UNIQUE (name)
);

-- Table: qgis_test.books_authors


CREATE TABLE qgis_test.books_authors
(
  fk_book integer NOT NULL,
  fk_author integer NOT NULL,
  CONSTRAINT books_authors_pkey PRIMARY KEY (fk_book, fk_author),
  CONSTRAINT books_authors_fk_author_fkey FOREIGN KEY (fk_author)
      REFERENCES qgis_test.authors (pk) MATCH SIMPLE
      ON UPDATE NO ACTION ON DELETE CASCADE,
  CONSTRAINT books_authors_fk_book_fkey FOREIGN KEY (fk_book)
      REFERENCES qgis_test.books (pk) MATCH SIMPLE
      ON UPDATE NO ACTION ON DELETE CASCADE
);

INSERT INTO qgis_test.authors(name)
	    VALUES 
              ('Erich Gamma'),
              ('Richard Helm'),
              ('Ralph Johnson'),
              ('John Vlissides'),
              ('Douglas Adams'),
              ('Ken Follett'),
              ('Gabriel García Márquez');

INSERT INTO qgis_test.books(name)
	    VALUES
              ('Design Patterns. Elements of Reusable Object-Oriented Software');

INSERT INTO qgis_test.books_authors(fk_book, fk_author)
	    VALUES 
              (1, 1),
              (1, 2),
              (1, 3),
              (1, 4);
