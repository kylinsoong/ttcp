# Use openjdk 13 as a base image
FROM openjdk:11

# Set Label
LABEL maintainer "kylinsoong.1214@gmail.com"

# Set the working directory to /app
WORKDIR /app

# Copy the runable jar into the container at /app
ADD target/echo-service-1.0.jar /app

# Make port 8080 available to the world outside this container
EXPOSE 8877

ENV APP_DATACENTER_NAME="local"
ENV APP_VERSION_NUMBER="0.0"

# Run  when the container launches
CMD ["java", "-jar", "echo-service-1.0.jar"]
