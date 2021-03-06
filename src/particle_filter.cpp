#include <random>
#include <algorithm>
#include <iostream>

#include "particle_filter.h"
#include "helper_functions.h"

using namespace std;

void ParticleFilter::init(double x, double y, double theta, double std[]) {
    // TODO: Set the number of particles. Initialize all particles to first position (based on estimates of
    //   x, y, theta and their uncertainties from GPS) and all weights to 1.
    // Add random Gaussian noise to each particle.
    // NOTE: Consult particle_filter.h for more information about this method (and others in this file).
    num_particles = 100;
    particles.resize((unsigned long) num_particles);
    weights.resize((unsigned long) num_particles);


    default_random_engine gen;
    normal_distribution<double> N_x_init(x, std[0]);
    normal_distribution<double> N_y_init(y, std[1]);
    normal_distribution<double> N_theta_init(theta, std[2]);

    for(int i = 0; i< num_particles; ++i){

        particles[i].id	= i;
        particles[i].x = N_x_init(gen);
        particles[i].y =  N_y_init(gen);
        particles[i].theta = N_theta_init(gen);
        particles[i].weight = 1;

        weights[i] = 1;
    }
    is_initialized = true;
}

void ParticleFilter::prediction(double delta_t, double std_pos[], double velocity, double yaw_rate) {
    // TODO: Add measurements to each particle and add random Gaussian noise.
    // NOTE: When adding noise you may find std::normal_distribution and std::default_random_engine useful.
    //  http://en.cppreference.com/w/cpp/numeric/random/normal_distribution
    //  http://www.cplusplus.com/reference/random/default_random_engine/

    random_device rd;
    default_random_engine gen(rd());

    for(int i = 0; i< num_particles; ++i){

        double next_x = particles[i].x + (velocity/yaw_rate)*(sin(particles[i].theta + yaw_rate*delta_t) - sin(particles[i].theta));
        double next_y = particles[i].y +  (velocity/yaw_rate)*(cos(particles[i].theta)- cos(particles[i].theta + yaw_rate*delta_t));
        double next_theta = particles[i].theta + yaw_rate*delta_t;

        normal_distribution<double> N_x(next_x, std_pos[0]);
        normal_distribution<double> N_y(next_y, std_pos[1]);
        normal_distribution<double> N_theta(next_theta, std_pos[2]);

        particles[i].x = N_x(gen);
        particles[i].y = N_y(gen);
        particles[i].theta = N_theta(gen);
    }
}

void ParticleFilter::dataAssociation(std::vector<LandmarkObs> predicted, std::vector<LandmarkObs>& observations) {
    // TODO: Find the predicted measurement that is closest to each observed measurement and assign the
    //   observed measurement to this particular landmark.
    // NOTE: this method will NOT be called by the grading code. But you will probably find it useful to
    //   implement this method and use it as a helper during the updateWeights phase.
}

void ParticleFilter::updateWeights(double sensor_range, double std_landmark[],
                                   std::vector<LandmarkObs> observations, Map map_landmarks) {
    // TODO: Update the weights of each particle using a multi-variate Gaussian distribution. You can read
    //   more about this distribution here: https://en.wikipedia.org/wiki/Multivariate_normal_distribution
    // NOTE: The observations are given in the VEHICLE'S coordinate system. Your particles are located
    //   according to the MAP'S coordinate system. You will need to transform between the two systems.
    //   Keep in mind that this transformation requires both rotation AND translation (but no scaling).
    //   The following is a good resource for the theory:
    //   https://www.willamette.edu/~gorr/classes/GeneralGraphics/Transforms/transforms2d.htm
    //   and the following is a good resource for the actual equation to implement (look at equation
    //   3.33. Note that you'll need to switch the minus sign in that equation to a plus to account
    //   for the fact that the map's y-axis actually points downwards.)
    //   http://planning.cs.uiuc.edu/node99.html

    double sum_weights = 0;

    for(int i = 0; i < num_particles; ++i){

        double particle_weight = 1;

        for(int j = 0; j<observations.size(); ++j){
            double new_x = observations[j].x*cos(particles[i].theta) - observations[j].y*sin(particles[i].theta) + particles[i].x;
            double new_y = observations[j].x*sin(particles[i].theta) + observations[j].y*cos(particles[i].theta) + particles[i].y;

            Map::single_landmark_s closest_observations;
            double min_distance = sensor_range;
            double current_distance;

            for(int k = 0; k< map_landmarks.landmark_list.size();++k){
                current_distance = dist( map_landmarks.landmark_list[k].x_f, map_landmarks.landmark_list[k].y_f, new_x, new_y);
                if(current_distance<min_distance){
                    min_distance = current_distance;
                    closest_observations = map_landmarks.landmark_list[k];
                }
            }
            if (min_distance < sensor_range) {
                particle_weight *= exp(-(pow(new_x-closest_observations.x_f,2)/(2*pow(std_landmark[0],2)) + (pow(new_y-closest_observations.y_f,2))/(2*pow(std_landmark[1],2))))/(sqrt(2*M_PI*std_landmark[0]*std_landmark[1]));
            }
        }
        particles[i].weight = particle_weight;
        sum_weights += particle_weight;
    }

    for(int i = 0; i< num_particles; ++i){
        weights[i] = particles[i].weight/sum_weights;
    }
}

void ParticleFilter::resample() {
    // TODO: Resample particles with replacement with probability proportional to their weight.
    // NOTE: You may find std::discrete_distribution helpful here.
    //   http://en.cppreference.com/w/cpp/numeric/random/discrete_distribution
    random_device rd;
    default_random_engine gen(rd());
    discrete_distribution<> d(weights.begin(),weights.end());
    vector<Particle> resampled_particles(num_particles);

    for(int i = 0; i<num_particles; ++i){
        resampled_particles[i] = particles[d(gen)];
    }

    particles = resampled_particles;
}

void ParticleFilter::write(std::string filename) {
    // You don't need to modify this file.
    std::ofstream dataFile;
    dataFile.open(filename, std::ios::app);
    for (int i = 0; i < num_particles; ++i) {
        dataFile << particles[i].x << " " << particles[i].y << " " << particles[i].theta << "\n";
    }
    dataFile.close();
}
