/*
 * particle_filter.cpp
 *
 *  Created on: Dec 12, 2016
 *      Author: Tiffany Huang
 *
 *  Revised on: April 18, 2017
 *      Auther: Yasen Hu
 */

#include <random>
#include <algorithm>
#include <iostream>
#include <numeric>

#include "particle_filter.h"

void ParticleFilter::init(double x, double y, double theta, double std[]) {
	// Set the number of particles. Initialize all particles to first position (based on estimates of
	//  x, y, theta and their uncertainties from GPS) and all weights to 1.
	// Add random Gaussian noise to each particle.
	// NOTE: Consult particle_filter.h for more information about this method (and others in this file).

    num_particles = 200;

    std::default_random_engine gen;
    // This line creates a normal (Gaussian) distribution for x, y and theta
    std::normal_distribution<double> dist_x(0, std[0]);
    std::normal_distribution<double> dist_y(0, std[1]);
    std::normal_distribution<double> dist_theta(0, std[2]);

    for (int i = 0; i < num_particles; i++) {
        double sample_x, sample_y, sample_theta;

        // Sample  and from these normal distrubtions like this:
        // sample_x = dist_x(gen);
        // where "gen" is the random engine initialized earlier.
        sample_x = dist_x(gen);
        sample_y = dist_y(gen);
        sample_theta = dist_theta(gen);

        Particle particle;
        particle.id = i;
        particle.x = x + sample_x;
        particle.y = y + sample_y;
        particle.theta = theta + sample_theta;
        particle.weight = 1.0;

        weights.push_back(particle.weight);
        particles.push_back(particle);
    }

    // initialization done
    is_initialized = true;
}

void ParticleFilter::prediction(double delta_t, double std_pos[], double velocity, double yaw_rate) {
	// Add measurements to each particle and add random Gaussian noise.
	// NOTE: When adding noise you may find std::normal_distribution and std::default_random_engine useful.
	// http://en.cppreference.com/w/cpp/numeric/random/normal_distribution
	// http://www.cplusplus.com/reference/random/default_random_engine/

    std::default_random_engine gen;

    // This line creates a normal (Gaussian) distribution for x, y and theta
    std::normal_distribution<double> dist_x(0, std_pos[0]);
    std::normal_distribution<double> dist_y(0, std_pos[1]);
    std::normal_distribution<double> dist_theta(0, std_pos[2]);

    for (int i = 0; i < num_particles; i++) {
        //avoid division by zero
        if (fabs(yaw_rate) > 0.001) {
            particles[i].x += (velocity / yaw_rate) * (sin(particles[i].theta + yaw_rate * delta_t) - sin(particles[i].theta));
            particles[i].y += (velocity / yaw_rate) * (cos(particles[i].theta) - cos(particles[i].theta + yaw_rate * delta_t));
            particles[i].theta += yaw_rate * delta_t;
        }
        else {
            particles[i].x += velocity * delta_t * cos(particles[i].theta);
            particles[i].y += velocity * delta_t * sin(particles[i].theta);
        }

        particles[i].x +=  dist_x(gen);
        particles[i].y +=  dist_y(gen);
        particles[i].theta += dist_theta(gen);
    }
}

void ParticleFilter::dataAssociation(std::vector<LandmarkObs> predicted, std::vector<LandmarkObs>& observations) {
	// Find the predicted measurement that is closest to each observed measurement and assign the
	//  observed measurement to this particular landmark.
	// NOTE: this method will NOT be called by the grading code. But you will probably find it useful to 
	//  implement this method and use it as a helper during the updateWeights phase.

    for (int i = 0; i < observations.size(); i++) {
        double min_dist = 1e8;
        int min_index = -1;

        for (int j = 0; j < predicted.size(); j++) {
            double cur_dist = dist(observations[i].x, observations[i].y, predicted[j].x, predicted[j].y);
            if (cur_dist < min_dist) {
                min_dist = cur_dist;
                min_index = j;
            }
        }
        // assign nearest neighbor
        observations[i].id = predicted[min_index].id;
        // play some trick here, use delta distance instead of real distance
        observations[i].x -= predicted[min_index].x;
        observations[i].y -= predicted[min_index].y;
    }
}

void ParticleFilter::updateWeights(double sensor_range, double std_landmark[], 
		std::vector<LandmarkObs> observations, Map map_landmarks) {
	// Update the weights of each particle using a mult-variate Gaussian distribution. You can read
	//   more about this distribution here: https://en.wikipedia.org/wiki/Multivariate_normal_distribution
	// NOTE: The observations are given in the VEHICLE'S coordinate system. Your particles are located
	//   according to the MAP'S coordinate system. You will need to transform between the two systems.
	// Keep in mind that this transformation requires both rotation AND translation (but no scaling).
	// The following is a good resource for the theory:
	// https://www.willamette.edu/~gorr/classes/GeneralGraphics/Transforms/transforms2d.htm
	//   and the following is a good resource for the actual equation to implement (look at equation 
	//   3.33. Note that you'll need to switch the minus sign in that equation to a plus to account 
	//   for the fact that the map's y-axis actually points downwards.)
	// http://planning.cs.uiuc.edu/node99.html

    // initial weights
    weights.clear();
    // update every particle
    for (int i = 0; i < num_particles; i++) {
        std::vector<LandmarkObs> landmarks_on_map;
        std::vector<LandmarkObs> obs_on_map;

        // find possible landmarks in range
        for (int j = 0; j < map_landmarks.landmark_list.size(); j++) {
            double cur_dist = dist(map_landmarks.landmark_list[j].x_f, map_landmarks.landmark_list[j].y_f,
                                   particles[j].x, particles[j].y);
            if (cur_dist <= sensor_range) {
                LandmarkObs landmark;
                landmark.id = map_landmarks.landmark_list[j].id_i;
                landmark.x = map_landmarks.landmark_list[j].x_f;
                landmark.y = map_landmarks.landmark_list[j].y_f;

                landmarks_on_map.push_back(landmark);
            }
        }

        // convert observation to map's coordinate system
        for (int j = 0; j < observations.size(); j++) {
            double cur_dist = dist(observations[j].x, observations[j].y, 0, 0);
            if (cur_dist <= sensor_range) {
                LandmarkObs landmark;
                landmark.id = -1;
                landmark.x = particles[i].x + observations[j].x * cos(particles[i].theta) - observations[j].y * sin(particles[i].theta);
                landmark.y = particles[i].y + observations[j].x * sin(particles[i].theta) + observations[j].y * cos(particles[i].theta);

                obs_on_map.push_back(landmark);
            }
        }

        dataAssociation(landmarks_on_map, obs_on_map);

        // initial weight
        double weight = 1.0;
        double sx = std_landmark[0];
        double sy = std_landmark[1];

        // according to https://en.wikipedia.org/wiki/Multivariate_normal_distribution
        // Bivariate case (assume ρ = 0, which is the correlation between X and Y)
        for (int j = 0; j < obs_on_map.size(); j++){
            double dx = obs_on_map[j].x;
            double dy = obs_on_map[j].y;

            weight *= (1.0 / (2.0 * M_PI * sx * sy)) * exp(-0.5 * ((dx * dx) / (sx * sx) + (dy * dy) / (sy * sy)));
        }

        weights.push_back(weight);
        particles[i].weight = weight;

    }
}

void ParticleFilter::resample() {
	// Resample particles with replacement with probability proportional to their weight.
	// NOTE: You may find std::discrete_distribution helpful here.
	// http://en.cppreference.com/w/cpp/numeric/random/discrete_distribution

    std::default_random_engine gen;
    std::discrete_distribution<int> dist(weights.cbegin(), weights.cend());

    // Make a copy of current particles
    std::vector<Particle> temp_particles = particles;
    particles.clear();

    for (int i = 0; i < num_particles; i++) {
        Particle particle;
        int index = dist(gen);
        particle = temp_particles[index];
        particles.push_back(particle);
    }
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
