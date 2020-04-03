/**
 * particle_filter.cpp
 *
 * Created on: Dec 12, 2016
 * Author: Tiffany Huang, François-Guillaume Fernandez
 */

#include "particle_filter.h"

#include <math.h>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include "helpers.h"

using std::cos;
using std::sin;
using std::string;
using std::vector;

void ParticleFilter::init(double x, double y, double theta, double std[]) {
  /**
   * Initialize particle around a given state.
   */
  if (!initialized()) {
    num_particles = 100;
    particles.resize(num_particles);

    // Gausian distributions
    std::normal_distribution<double> dist_x(x, std[0]);
    std::normal_distribution<double> dist_y(y, std[1]);
    std::normal_distribution<double> dist_theta(theta, std[2]);

    // Initialize all particles
    std::default_random_engine gen;
    for (uint i = 0; i < num_particles; i++) {
      particles[i].id = i;
      particles[i].x = dist_x(gen);
      particles[i].y = dist_y(gen);
      particles[i].theta = dist_theta(gen);
      particles[i].weight = 1.0;
    }
    is_initialized = true;
  }
}

void ParticleFilter::prediction(double delta_t, double std_pos[],
                                double velocity, double yaw_rate) {
  /**
   * Updates particle predictions
   */

  // Gausian distributions
  // distribution remains the same by adding the mean to a 0-centered one
  std::normal_distribution<double> dist_x(0, std_pos[0]);
  std::normal_distribution<double> dist_y(0, std_pos[1]);
  std::normal_distribution<double> dist_theta(0, std_pos[2]);
  std::default_random_engine gen;

  for (uint i = 0; i < particles.size(); i++) {
    // Convert from car coordinates to map coordinates
    particles[i].x += velocity * cos(particles[i].theta) * delta_t;
    particles[i].y += velocity * sin(particles[i].theta) * delta_t;
    particles[i].theta += yaw_rate * delta_t;

    // Add noise
    particles[i].x += dist_x(gen);
    particles[i].y += dist_y(gen);
    particles[i].theta += dist_theta(gen);
  }
}

void ParticleFilter::dataAssociation(vector<LandmarkObs> predicted,
                                     vector<LandmarkObs>& observations) {
  /**
   * Associate observations to predicted landmarks
   */
  double dist2_, min_dist2;
  for (uint i = 0; i < observations.size(); i++) {
    min_dist2 = std::numeric_limits<double>::infinity();
    for (uint j = 0; j < predicted.size(); j++) {
      // No need to take the squared root as the order is the same
      dist2_ = dist2(observations[i].x, observations[i].y, predicted[j].x,
                     predicted[j].y);

      if (dist2_ < min_dist2) {
        min_dist2 = dist2_;
        observations[i].id = predicted[j].id;
      }
    }
  }
}

void ParticleFilter::updateWeights(double sensor_range, double std_landmark[],
                                   const vector<LandmarkObs>& observations,
                                   const Map& map_landmarks) {
  /**
   * For each particle, convert observations to map coordinates, filter
   * landmarks that are within sensor range, associate observations with
   * predicted landmarks, and reweight the particle using a Gaussian
   * distribution
   */
  double dist2_, sr2;
  double varx = 2 * pow(std_landmark[0], 2);
  double vary = 2 * pow(std_landmark[1], 2);
  double normalization_term =
      1 / (2 * M_PI * std_landmark[0] * std_landmark[1]);
  double cos_theta, sin_theta, diff_x, diff_y;
  double w_sum;
  sr2 = sensor_range * sensor_range;

  for (uint i = 0; i < num_particles; i++) {
    vector<LandmarkObs> converted_observations;
    converted_observations.resize(observations.size());
    cos_theta = cos(particles[i].theta);
    sin_theta = sin(particles[i].theta);
    // Convert to map coordinated
    for (uint j = 0; j < observations.size(); j++) {
      converted_observations[j].id = j;
      converted_observations[j].x = particles[i].x +
                                    cos_theta * observations[j].x -
                                    sin_theta * observations[j].y;
      converted_observations[j].y = particles[i].y +
                                    sin_theta * observations[j].x +
                                    cos_theta * observations[j].y;
    }

    // Keep only landmarks in sensor range of the current particle
    vector<LandmarkObs> observable_landmarks;
    for (uint j = 0; j < map_landmarks.landmark_list.size(); j++) {
      dist2_ = dist2(particles[i].x, particles[i].y,
                     map_landmarks.landmark_list[j].x_f,
                     map_landmarks.landmark_list[j].y_f);
      if (dist2_ <= sr2) {
        observable_landmarks.push_back(
            LandmarkObs{map_landmarks.landmark_list[j].id_i,
                        map_landmarks.landmark_list[j].x_f,
                        map_landmarks.landmark_list[j].y_f});
      }
    }

    // Associate observations with predicted landmarks using nearest neighbours
    dataAssociation(observable_landmarks, converted_observations);

    // Compute the weight for each particle
    particles[i].weight = 1.0;
    for (uint j = 0; j < converted_observations.size(); j++) {
      for (uint k = 0; k < observable_landmarks.size(); k++) {
        if (converted_observations[j].id == observable_landmarks[k].id) {
          diff_x = observable_landmarks[k].x - converted_observations[j].x;
          diff_y = observable_landmarks[k].y - converted_observations[j].y;
          particles[i].weight *=
              normalization_term *
              exp(-(pow(diff_x, 2) / varx + pow(diff_y, 2) / vary));
        }
      }
    }
    w_sum += particles[i].weight;
  }

  // Normalize weights
  for (uint i = 0; i < particles.size(); i++) {
    particles[i].weight /= w_sum;
  }
}

void ParticleFilter::resample() {
  /**
   * Resample particles with replacement probability proportional to their
   * weight
   */

  vector<Particle> resampled_particles;
  resampled_particles.resize(num_particles);
  std::uniform_int_distribution<> ddist(0, particles.size() - 1);
  std::default_random_engine gen;
  uint index = ddist(gen);
  double beta = 0;
  // Find the biggest weight
  double max_weight = 0;
  for (uint i = 0; i < particles.size(); i++) {
    if (particles[i].weight > max_weight) {
      max_weight = particles[i].weight;
    }
  }

  // Wheel trick to resample the particles
  std::uniform_real_distribution<double> random_weight(0.0, 2 * max_weight);

  for (uint i = 0; i < num_particles; i++) {
    beta += random_weight(gen);
    while (beta > particles[index].weight) {
      beta -= particles[index].weight;
      index = (index + 1) % num_particles;
    }
    resampled_particles[i] = particles[index];
  }
  particles = resampled_particles;
}

void ParticleFilter::SetAssociations(Particle& particle,
                                     const vector<int>& associations,
                                     const vector<double>& sense_x,
                                     const vector<double>& sense_y) {
  // particle: the particle to which assign each listed association,
  //   and association's (x,y) world coordinates mapping
  // associations: The landmark id that goes along with each listed association
  // sense_x: the associations x mapping already converted to world coordinates
  // sense_y: the associations y mapping already converted to world coordinates
  particle.associations = associations;
  particle.sense_x = sense_x;
  particle.sense_y = sense_y;
}

string ParticleFilter::getAssociations(Particle best) {
  vector<int> v = best.associations;
  std::stringstream ss;
  copy(v.begin(), v.end(), std::ostream_iterator<int>(ss, " "));
  string s = ss.str();
  s = s.substr(0, s.length() - 1);
  return s;
}

string ParticleFilter::getSenseCoord(Particle best, string coord) {
  vector<double> v;

  if (coord == "X") {
    v = best.sense_x;
  } else {
    v = best.sense_y;
  }

  std::stringstream ss;
  copy(v.begin(), v.end(), std::ostream_iterator<float>(ss, " "));
  string s = ss.str();
  s = s.substr(0, s.length() - 1);
  return s;
}
