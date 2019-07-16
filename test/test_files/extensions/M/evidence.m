function [net, gamma, logev] = evidence(net, x, t, num)
%EVIDENCE Re-estimate hyperparameters using evidence approximation.
%
%	Description
%	[NET] = EVIDENCE(NET, X, T) re-estimates the hyperparameters ALPHA
%	and BETA by applying Bayesian re-estimation formulae for NUM
%	iterations. The hyperparameter ALPHA can be a simple scalar
%	associated with an isotropic prior on the weights, or can be a vector
%	in which each component is associated with a group of weights as
%	defined by the INDEX matrix in the NET data structure. These more
%	complex priors can be set up for an MLP using MLPPRIOR. Initial
%	values for the iterative re-estimation are taken from the network
%	data structure NET passed as an input argument, while the return
%	argument NET contains the re-estimated values.
%
%	[NET, GAMMA, LOGEV] = EVIDENCE(NET, X, T, NUM) allows the re-
%	estimation  formula to be applied for NUM cycles in which the re-
%	estimated values for the hyperparameters from each cycle are used to
%	re-evaluate the Hessian matrix for the next cycle.  The return value
%	GAMMA is the number of well-determined parameters and LOGEV is the
%	log of the evidence.
%
%	See also
%	MLPPRIOR, NETGRAD, NETHESS, DEMEV1, DEMARD
%

%	Copyright (c) Ian T Nabney (1996-2001)

errstring = consist(net, '', x, t);
if ~isempty(errstring)
  error(errstring);
end

ndata = size(x, 1);
if nargin == 3
  num = 1;
end

% Extract weights from network
w = netpak(net);

% Evaluate data-dependent contribution to the Hessian matrix.
[h, dh] = nethess(w, net, x, t); 
clear h;  % To save memory when Hessian is large
if (~isfield(net, 'beta'))
  local_beta = 1;
end

[evec, evl] = eig(dh);
% Now set the negative eigenvalues to zero.
evl = evl.*(evl > 0);
% safe_evl is used to avoid taking log of zero
safe_evl = evl + eps.*(evl <= 0);

[e, edata, eprior] = neterr(w, net, x, t);

if size(net.alpha) == [1 1]
  % Form vector of eigenvalues
  evl = diag(evl);
  safe_evl = diag(safe_evl);
else
  ngroups = size(net.alpha, 1);
  gams = zeros(1, ngroups);
  logas = zeros(1, ngroups);
  % Reconstruct data hessian with negative eigenvalues set to zero.
  dh = evec*evl*evec';
end

% Do the re-estimation. 
for k = 1 : num
  % Re-estimate alpha.
  if size(net.alpha) == [1 1]
    % Evaluate number of well-determined parameters.
    L = evl;
    if isfield(net, 'beta')
      L = net.beta*L;
    end
    gamma = sum(L./(L + net.alpha));
    net.alpha = 0.5*gamma/eprior;
    % Partially evaluate log evidence: only include unmasked weights
    logev = 0.5*length(w)*log(net.alpha);
  else
    hinv = inv(hbayes(net, dh));
    for m = 1 : ngroups
      group_nweights = sum(net.index(:, m));
      gams(m) = group_nweights - ...
	        net.alpha(m)*sum(diag(hinv).*net.index(:,m));
      net.alpha(m) = real(gams(m)/(2*eprior(m)));
      % Weight alphas by number of weights in group
      logas(m) = 0.5*group_nweights*log(net.alpha(m));
    end 
    gamma = sum(gams, 2);
    logev = sum(logas);
  end
  % Re-estimate beta.
  if isfield(net, 'beta')
      net.beta = 0.5*(net.nout*ndata - gamma)/edata;
      logev = logev + 0.5*ndata*log(net.beta) - 0.5*ndata*log(2*pi);
      local_beta = net.beta;
  end
  
  % Evaluate new log evidence
  e = errbayes(net, edata);
  if size(net.alpha) == [1 1]
    logev = logev - e - 0.5*sum(log(local_beta*safe_evl+net.alpha));
  else
    for m = 1:ngroups  
      logev = logev - e - ...
	  0.5*sum(log(local_beta*(safe_evl*net.index(:, m))+...
	  net.alpha(m)));
    end
  end
end

