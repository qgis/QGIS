/**
 * @file main.cpp
 * @brief Main file.
 * @author Michał Policht
 */

#include <qextserialenumerator.h>

int main(int argc, char *argv[])
{
	QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
	printf("List of ports:\n");
	for (int i = 0; i < ports.size(); i++) {
		printf("port name: %s\n", ports.at(i).portName.toLocal8Bit().constData());
		printf("friendly name: %s\n", ports.at(i).friendName.toLocal8Bit().constData());
		printf("physical name: %s\n", ports.at(i).physName.toLocal8Bit().constData());
		printf("enumerator name: %s\n", ports.at(i).enumName.toLocal8Bit().constData());
		printf("===================================\n\n");
	}
	return EXIT_SUCCESS;
}
