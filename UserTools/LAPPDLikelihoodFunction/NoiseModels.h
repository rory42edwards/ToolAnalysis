#ifndef NoiseModel_H
#define NoiseModel_H

class NoiseModel {
    public:
        virtual double LogLikelihood(double residual) = 0;
        virtual ~NoiseModel() {}
};

class GaussianNoise : public NoiseModel {
    double sigma;
    public:
        GaussianNoise(double sigma) : sigma(sigma) {}
        double LogLikelihood(double residual) override {
            return (residual * residual) / (2 * sigma * sigma);
        }
};

#endif
